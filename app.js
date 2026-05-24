/* 济事楼导览 —— 交互逻辑 */
(function () {
  'use strict';

  const SVG_NS = 'http://www.w3.org/2000/svg';
  const ROOM_MAP = Object.fromEntries(ROOM_DATA.map((r) => [r.id, r]));

  const CAT_VARS = {
    office:   'var(--c-office)',
    teaching: 'var(--c-teaching)',
    research: 'var(--c-research)',
    service:  'var(--c-service)',
    unknown:  'var(--c-unknown)',
    blocked:  'var(--c-blocked)'
  };

  const FIXTURE_GLYPH = {
    stairs:   '🪜',
    elevator: '🛗',
    bathroom: '🚻',
    blocked:  '⛔'
  };
  const FIXTURE_LABEL = {
    stairs:   '楼梯',
    elevator: '电梯',
    bathroom: '卫生间',
    blocked:  '此路不通'
  };

  const state = {
    floor: '4',
    view: 'plan',
    categories: new Set(),
    keyword: '',
    selectedId: null
  };

  const $ = (sel) => document.querySelector(sel);
  const searchInput    = $('#searchInput');
  const searchClear    = $('#searchClear');
  const floorSections  = $('#floorSections');
  const detailPanel    = $('#detailPanel');
  const categoryFilter = $('#categoryFilter');
  const resultCount    = $('#resultCount');
  const emptyHint      = $('#emptyHint');
  const planView       = $('#planView');
  const listView       = $('#listView');
  const planTitle      = $('#planTitle');
  const planMeta       = $('#planMeta');
  const floorPlanSvg   = $('#floorPlan');
  const tooltip        = $('#tooltip');

  const CN_NUM = ['零','一','二','三','四','五','六','七','八','九'];

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, (c) => ({
      '&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'
    }[c]));
  }

  function highlight(text) {
    const safe = escapeHtml(text);
    if (!state.keyword) return safe;
    const kw = state.keyword.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    return safe.replace(new RegExp('(' + kw + ')', 'gi'), '<mark>$1</mark>');
  }

  function staffText(s) {
    return [s.name, s.title, s.note].filter(Boolean).join(' ');
  }

  function matchesKeyword(room) {
    if (!state.keyword) return true;
    const haystack = [
      room.id, room.name, room.purpose,
      room.staff.map(staffText).join(' '),
      room.tags.join(' '),
      (CATEGORY_META[room.category] || { label: '' }).label
    ].join(' ').toLowerCase();
    return haystack.indexOf(state.keyword) !== -1;
  }

  function passesFilter(room) {
    if (state.categories.size && !state.categories.has(room.category)) return false;
    return matchesKeyword(room);
  }

  function getFloorRooms(floor) {
    return ROOM_DATA.filter((r) => String(r.floor) === String(floor));
  }

  // ============== 分类筛选 ==============
  function renderCategoryFilter() {
    // 清空除 label 以外的子元素
    [...categoryFilter.children].forEach((c) => {
      if (!c.classList.contains('nav-label')) c.remove();
    });
    ['office','teaching','research','service'].forEach((key) => {
      const meta = CATEGORY_META[key];
      const btn = document.createElement('button');
      btn.className = 'nav-link' + (state.categories.has(key) ? ' is-active' : '');
      btn.textContent = meta.label;
      btn.addEventListener('click', () => {
        if (state.categories.has(key)) state.categories.delete(key);
        else state.categories.add(key);
        render();
      });
      categoryFilter.appendChild(btn);
    });
  }

  // ============== SVG 平面图 ==============
  function renderFixture(svg, fx) {
    const g = document.createElementNS(SVG_NS, 'g');
    g.setAttribute('class', 'fixture fixture-' + fx.type);
    const r = document.createElementNS(SVG_NS, 'rect');
    r.setAttribute('x', fx.x); r.setAttribute('y', fx.y);
    r.setAttribute('width', fx.w); r.setAttribute('height', fx.h);
    g.appendChild(r);
    const cx = fx.x + fx.w / 2;
    const glyph = FIXTURE_GLYPH[fx.type];
    const labelText = fx.label || FIXTURE_LABEL[fx.type] || '';
    if (glyph && fx.h >= 28) {
      const g1 = document.createElementNS(SVG_NS, 'text');
      g1.setAttribute('x', cx);
      g1.setAttribute('y', fx.y + fx.h / 2 - 1);
      g1.setAttribute('class', 'fixture-glyph');
      g1.textContent = glyph;
      g.appendChild(g1);
      const lbl = document.createElementNS(SVG_NS, 'text');
      lbl.setAttribute('x', cx);
      lbl.setAttribute('y', fx.y + fx.h - 4);
      lbl.setAttribute('class', 'fixture-label');
      lbl.textContent = labelText;
      g.appendChild(lbl);
    } else {
      const lbl = document.createElementNS(SVG_NS, 'text');
      lbl.setAttribute('x', cx);
      lbl.setAttribute('y', fx.y + fx.h / 2 + 4);
      lbl.setAttribute('class', 'fixture-label');
      lbl.textContent = labelText;
      g.appendChild(lbl);
    }
    svg.appendChild(g);
  }

  function renderPlan() {
    const layout = FLOOR_LAYOUTS[state.floor];
    floorPlanSvg.innerHTML = '';
    floorPlanSvg.setAttribute('viewBox', layout.viewBox);
    planTitle.textContent = '济事楼' + CN_NUM[+state.floor] + '楼平面图';

    if (layout.outline) {
      const path = document.createElementNS(SVG_NS, 'path');
      path.setAttribute('d', layout.outline);
      path.setAttribute('class', 'floor-outline');
      floorPlanSvg.appendChild(path);
    }

    (layout.fixtures || []).forEach((fx) => renderFixture(floorPlanSvg, fx));

    let matchCount = 0;
    const totalCount = layout.rooms.length;

    layout.rooms.forEach((r) => {
      const room = ROOM_MAP[r.id];
      if (!room) return;

      const matched = passesFilter(room);
      const selected = state.selectedId === r.id;
      if (matched) matchCount++;

      const catColor = CAT_VARS[room.category] || CAT_VARS.unknown;
      const g = document.createElementNS(SVG_NS, 'g');
      g.setAttribute('class',
        'room' +
        (matched ? '' : ' is-dimmed') +
        (selected ? ' is-selected' : '') +
        (room.category === 'blocked' ? ' is-blocked' : '')
      );
      g.setAttribute('data-id', r.id);
      g.style.setProperty('--cat', catColor);

      let shape, cx, cy, bandY, bandX, bandW;
      if (r.rect) {
        const [x, y, w, h] = r.rect;
        shape = document.createElementNS(SVG_NS, 'rect');
        shape.setAttribute('x', x); shape.setAttribute('y', y);
        shape.setAttribute('width', w); shape.setAttribute('height', h);
        cx = x + w / 2; cy = y + h / 2;
        bandX = x; bandY = y; bandW = w;
      } else {
        shape = document.createElementNS(SVG_NS, 'polygon');
        shape.setAttribute('points', r.polygon);
        const pts = r.polygon.split(/\s+/).map((p) => p.split(',').map(Number));
        const xs = pts.map((p) => p[0]);
        const ys = pts.map((p) => p[1]);
        cx = (Math.min(...xs) + Math.max(...xs)) / 2;
        cy = (Math.min(...ys) + Math.max(...ys)) / 2;
        bandX = Math.min(...xs); bandY = Math.min(...ys);
        bandW = Math.max(...xs) - bandX;
      }
      shape.setAttribute('class', 'room-shape');
      g.appendChild(shape);

      // 分类色条（顶部 3px 细条）
      if (room.category !== 'blocked') {
        const band = document.createElementNS(SVG_NS, 'rect');
        band.setAttribute('x', bandX);
        band.setAttribute('y', bandY);
        band.setAttribute('width', bandW);
        band.setAttribute('height', 3);
        band.setAttribute('class', 'room-cat-band');
        g.appendChild(band);
      }

      const label = document.createElementNS(SVG_NS, 'text');
      label.setAttribute('x', cx);
      label.setAttribute('y', cy + 4);
      label.setAttribute('class', 'room-label');
      label.textContent = r.id;
      g.appendChild(label);

      g.addEventListener('click', () => selectRoom(r.id));
      g.addEventListener('mouseenter', (e) => showTooltip(e, room));
      g.addEventListener('mousemove', moveTooltip);
      g.addEventListener('mouseleave', hideTooltip);
      g.setAttribute('tabindex', '0');
      g.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); selectRoom(r.id); }
      });

      floorPlanSvg.appendChild(g);
    });

    const showFilter = state.keyword || state.categories.size;
    planMeta.textContent = showFilter
      ? '本层共 ' + totalCount + ' 间，符合条件 ' + matchCount + ' 间'
      : '本层共 ' + totalCount + ' 间';
  }

  // ============== 列表视图（表格式样） ==============
  function renderList() {
    const rooms = getFloorRooms(state.floor).filter(passesFilter);
    floorSections.innerHTML = '';
    emptyHint.hidden = rooms.length !== 0;
    resultCount.textContent = '本层共 ' + rooms.length + ' 间';

    if (!rooms.length) return;

    const group = document.createElement('div');
    group.className = 'floor-group';
    const titleEl = document.createElement('div');
    titleEl.className = 'floor-group-title';
    titleEl.textContent = '济事楼' + CN_NUM[+state.floor] + '楼';
    group.appendChild(titleEl);

    const table = document.createElement('table');
    table.className = 'room-table';
    table.innerHTML =
      '<thead><tr>' +
        '<th class="col-id">房间号</th>' +
        '<th>房间名称</th>' +
        '<th class="col-cat">类别</th>' +
        '<th class="col-staff">主要人员</th>' +
      '</tr></thead>';
    const tbody = document.createElement('tbody');
    rooms.forEach((room) => {
      const meta = CATEGORY_META[room.category] || CATEGORY_META.unknown;
      const tr = document.createElement('tr');
      if (state.selectedId === room.id) tr.classList.add('is-selected');
      const names = room.staff.map((s) => s.name);
      const namesShort = names.slice(0, 4).join('、') +
        (names.length > 4 ? ' 等 ' + names.length + ' 人' : (names.length ? '' : '—'));
      tr.innerHTML =
        '<td class="col-id">' + highlight(room.id) + '</td>' +
        '<td>' + highlight(room.name) + '</td>' +
        '<td class="col-cat"><span class="cat-tag" style="--cat:' +
            CAT_VARS[room.category] + '">' + meta.label + '</span></td>' +
        '<td class="col-staff">' + highlight(namesShort) + '</td>';
      tr.addEventListener('click', () => selectRoom(room.id));
      tbody.appendChild(tr);
    });
    table.appendChild(tbody);
    group.appendChild(table);
    floorSections.appendChild(group);
  }

  // ============== 详情面板 ==============
  function renderDetail() {
    const room = ROOM_MAP[state.selectedId];
    if (!room) {
      detailPanel.innerHTML =
        '<div class="detail-empty">' +
          '<div class="empty-icon" aria-hidden="true">📍</div>' +
          '<p>请在左侧平面图中选择房间<br>查看用途与人员信息</p>' +
        '</div>';
      return;
    }
    const meta = CATEGORY_META[room.category] || CATEGORY_META.unknown;
    const catColor = CAT_VARS[room.category] || CAT_VARS.unknown;

    const staffHtml = room.staff.length
      ? '<div class="staff-list">' + room.staff.map((s) => {
          const titlePart = s.title ? '<span class="staff-title">' + highlight(s.title) + '</span>' : '';
          const notePart  = s.note  ? '<div class="staff-note">' + highlight(s.note) + '</div>' : '';
          return (
            '<div class="staff-item">' +
              '<span class="staff-name">' + highlight(s.name) + '</span>' +
              '<div class="staff-detail">' + titlePart + notePart + '</div>' +
            '</div>'
          );
        }).join('') + '</div>'
      : '<p class="staff-empty">暂无登记人员信息</p>';

    const tagsHtml = room.tags.length
      ? '<div class="dc-tags">' + room.tags.map((t) =>
          '<span class="dc-tag" data-tag="' + escapeHtml(t) + '">' + escapeHtml(t) + '</span>'
        ).join('') + '</div>'
      : '<p class="staff-empty">—</p>';

    detailPanel.innerHTML =
      '<div class="dc-header" style="--cat:' + catColor + '">' +
        '<div class="dc-id">济事楼 ' + room.id + ' 室</div>' +
        '<div class="dc-name">' + highlight(room.name) + '</div>' +
        '<span class="dc-cat-tag">' + meta.label + '</span>' +
      '</div>' +
      '<div class="dc-body">' +
        '<div class="dc-section"><h4>用途</h4><p>' + highlight(room.purpose) + '</p></div>' +
        '<div class="dc-section"><h4>人员</h4>' + staffHtml + '</div>' +
        '<div class="dc-section"><h4>开放时间</h4><p>' + escapeHtml(room.hours) + '</p></div>' +
        '<div class="dc-section"><h4>相关</h4>' + tagsHtml + '</div>' +
      '</div>';

    detailPanel.querySelectorAll('.dc-tag').forEach((tag) => {
      tag.addEventListener('click', () => {
        searchInput.value = tag.dataset.tag;
        state.keyword = tag.dataset.tag.toLowerCase();
        searchClear.classList.add('visible');
        render();
      });
    });
  }

  // ============== 悬停提示 ==============
  function showTooltip(e, room) {
    const meta = CATEGORY_META[room.category] || CATEGORY_META.unknown;
    const staffNames = room.staff.map((s) => s.name).slice(0, 4).join('、');
    tooltip.innerHTML =
      '<div class="tt-head">' + room.id + ' 室</div>' +
      '<div class="tt-body">' +
        '<div>' + escapeHtml(room.name) + '</div>' +
        '<div style="color:var(--ink-faint);font-size:12px;margin-top:2px;">' +
          escapeHtml(meta.label) + '</div>' +
        (staffNames ? '<div class="tt-staff">' + escapeHtml(staffNames) +
          (room.staff.length > 4 ? ' 等 ' + room.staff.length + ' 人' : '') + '</div>' : '') +
      '</div>';
    tooltip.hidden = false;
    moveTooltip(e);
  }
  function moveTooltip(e) {
    const pad = 14;
    let x = e.clientX + pad, y = e.clientY + pad;
    const rect = tooltip.getBoundingClientRect();
    if (x + rect.width  > window.innerWidth)  x = e.clientX - rect.width  - pad;
    if (y + rect.height > window.innerHeight) y = e.clientY - rect.height - pad;
    tooltip.style.left = x + 'px';
    tooltip.style.top  = y + 'px';
  }
  function hideTooltip() { tooltip.hidden = true; }

  // ============== 选择房间 ==============
  function selectRoom(id) {
    state.selectedId = id;
    const room = ROOM_MAP[id];
    if (room && String(room.floor) !== state.floor) {
      state.floor = String(room.floor);
      document.querySelectorAll('[data-floor]').forEach((b) => {
        b.classList.toggle('is-active', b.dataset.floor === state.floor);
      });
    }
    render();
    if (window.innerWidth <= 900) {
      detailPanel.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
  }

  // ============== 渲染入口 ==============
  function render() {
    renderCategoryFilter();
    if (state.view === 'plan') {
      planView.hidden = false;
      listView.hidden = true;
      renderPlan();
    } else {
      planView.hidden = true;
      listView.hidden = false;
      renderList();
    }
    renderDetail();
  }

  // ============== 事件 ==============
  document.querySelectorAll('[data-floor]').forEach((btn) => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('[data-floor]').forEach((b) => b.classList.remove('is-active'));
      btn.classList.add('is-active');
      state.floor = btn.dataset.floor;
      render();
    });
  });

  document.querySelectorAll('[data-view]').forEach((btn) => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('[data-view]').forEach((b) => b.classList.remove('is-active'));
      btn.classList.add('is-active');
      state.view = btn.dataset.view;
      render();
    });
  });

  searchInput.addEventListener('input', () => {
    state.keyword = searchInput.value.trim().toLowerCase();
    searchClear.classList.toggle('visible', searchInput.value.length > 0);
    if (state.keyword) {
      const hits = ROOM_DATA.filter(passesFilter);
      if (hits.length && !hits.some((r) => String(r.floor) === state.floor)) {
        state.floor = String(hits[0].floor);
        document.querySelectorAll('[data-floor]').forEach((b) => {
          b.classList.toggle('is-active', b.dataset.floor === state.floor);
        });
      }
    }
    render();
  });

  searchClear.addEventListener('click', () => {
    searchInput.value = '';
    state.keyword = '';
    searchClear.classList.remove('visible');
    searchInput.focus();
    render();
  });

  document.addEventListener('keydown', (e) => {
    if (e.key === '/' && document.activeElement !== searchInput) {
      e.preventDefault(); searchInput.focus();
    } else if (e.key === 'Escape' && state.selectedId) {
      state.selectedId = null; render();
    }
  });

  render();
})();
