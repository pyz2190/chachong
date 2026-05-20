/* 济事楼导览系统 —— 交互逻辑 */
(function () {
  'use strict';

  const SVG_NS = 'http://www.w3.org/2000/svg';
  const ROOM_MAP = Object.fromEntries(ROOM_DATA.map((r) => [r.id, r]));

  // ----- 应用状态 -----
  const state = {
    floor: '4',            // '4' | '5'
    view: 'plan',          // 'plan' | 'list'
    categories: new Set(), // 选中的分类，空表示全部
    keyword: '',           // 搜索关键字（小写）
    selectedId: null
  };

  // ----- DOM 引用 -----
  const $ = (sel) => document.querySelector(sel);
  const searchInput   = $('#searchInput');
  const searchClear   = $('#searchClear');
  const floorSections = $('#floorSections');
  const detailPanel   = $('#detailPanel');
  const categoryFilter= $('#categoryFilter');
  const resultCount   = $('#resultCount');
  const emptyHint     = $('#emptyHint');
  const planView      = $('#planView');
  const listView      = $('#listView');
  const planTitle     = $('#planTitle');
  const planMatchCount= $('#planMatchCount');
  const floorPlanSvg  = $('#floorPlan');
  const tooltip       = $('#tooltip');

  // ----- 工具函数 -----
  function escapeHtml(str) {
    return String(str).replace(/[&<>"']/g, (c) => ({
      '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
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

  // ============== 渲染：分类筛选 ==============
  function renderCategoryFilter() {
    categoryFilter.innerHTML = '';
    // 只展示主分类，不展示 unknown / blocked（不作为筛选维度）
    ['office', 'teaching', 'research', 'service'].forEach((key) => {
      const meta = CATEGORY_META[key];
      const chip = document.createElement('button');
      chip.className = 'cat-chip' + (state.categories.has(key) ? ' is-active' : '');
      chip.innerHTML = '<span class="dot" style="background:' + meta.color + '"></span>' + meta.label;
      chip.addEventListener('click', () => {
        if (state.categories.has(key)) state.categories.delete(key);
        else state.categories.add(key);
        render();
      });
      categoryFilter.appendChild(chip);
    });
  }

  // ============== 渲染：SVG 平面图 ==============
  function fixtureIcon(svg, fx) {
    const g = document.createElementNS(SVG_NS, 'g');
    g.setAttribute('class', 'fixture fixture-' + fx.type);
    const r = document.createElementNS(SVG_NS, 'rect');
    r.setAttribute('x', fx.x); r.setAttribute('y', fx.y);
    r.setAttribute('width', fx.w); r.setAttribute('height', fx.h);
    r.setAttribute('rx', 3);
    g.appendChild(r);
    const symbol = { stairs: '▲', elevator: '↕', bathroom: '🚻', blocked: '⊘' }[fx.type] || '';
    const t = document.createElementNS(SVG_NS, 'text');
    t.setAttribute('x', fx.x + fx.w / 2);
    t.setAttribute('y', fx.y + fx.h / 2 + 4);
    t.setAttribute('class', 'fixture-glyph');
    t.textContent = symbol;
    g.appendChild(t);
    if (fx.label) {
      const lbl = document.createElementNS(SVG_NS, 'text');
      lbl.setAttribute('x', fx.x + fx.w / 2);
      lbl.setAttribute('y', fx.y + fx.h + 14);
      lbl.setAttribute('class', 'fixture-label');
      lbl.textContent = fx.label;
      g.appendChild(lbl);
    }
    svg.appendChild(g);
  }

  function renderPlan() {
    const layout = FLOOR_LAYOUTS[state.floor];
    floorPlanSvg.innerHTML = '';
    floorPlanSvg.setAttribute('viewBox', layout.viewBox);
    planTitle.textContent = '济事楼 ' + state.floor + ' 楼平面图';

    // 楼层外轮廓
    if (layout.outline) {
      const path = document.createElementNS(SVG_NS, 'path');
      path.setAttribute('d', layout.outline);
      path.setAttribute('class', 'floor-outline');
      floorPlanSvg.appendChild(path);
    }

    // 公共设施（楼梯/电梯/卫生间/此路不通）
    (layout.fixtures || []).forEach((fx) => fixtureIcon(floorPlanSvg, fx));

    // 房间数量统计
    let matchCount = 0;
    const totalCount = layout.rooms.length;

    // 房间渲染
    layout.rooms.forEach((r) => {
      const room = ROOM_MAP[r.id];
      if (!room) return;

      const matched = passesFilter(room);
      const selected = state.selectedId === r.id;
      if (matched) matchCount++;

      const meta = CATEGORY_META[room.category] || CATEGORY_META.unknown;
      const g = document.createElementNS(SVG_NS, 'g');
      g.setAttribute('class',
        'room' +
        (matched ? '' : ' is-dimmed') +
        (selected ? ' is-selected' : '') +
        (room.category === 'blocked' ? ' is-blocked' : '')
      );
      g.setAttribute('data-id', r.id);

      let shape;
      let cx, cy;
      if (r.rect) {
        shape = document.createElementNS(SVG_NS, 'rect');
        shape.setAttribute('x', r.rect[0]);
        shape.setAttribute('y', r.rect[1]);
        shape.setAttribute('width', r.rect[2]);
        shape.setAttribute('height', r.rect[3]);
        shape.setAttribute('rx', 4);
        cx = r.rect[0] + r.rect[2] / 2;
        cy = r.rect[1] + r.rect[3] / 2;
      } else {
        shape = document.createElementNS(SVG_NS, 'polygon');
        shape.setAttribute('points', r.polygon);
        // 用平均点近似中心
        const pts = r.polygon.split(/\s+/).map((p) => p.split(',').map(Number));
        cx = pts.reduce((s, p) => s + p[0], 0) / pts.length;
        cy = pts.reduce((s, p) => s + p[1], 0) / pts.length;
      }
      shape.setAttribute('class', 'room-shape');
      shape.style.setProperty('--cat', meta.color);
      g.appendChild(shape);

      // 房间号
      const label = document.createElementNS(SVG_NS, 'text');
      label.setAttribute('x', cx);
      label.setAttribute('y', cy + 5);
      label.setAttribute('class', 'room-label');
      label.textContent = r.id;
      g.appendChild(label);

      // 交互
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

    planMatchCount.textContent = (state.keyword || state.categories.size)
      ? (matchCount + ' / ' + totalCount)
      : totalCount;
  }

  // ============== 渲染：列表视图 ==============
  function roomCardHtml(room) {
    const meta = CATEGORY_META[room.category] || CATEGORY_META.unknown;
    const selected = room.id === state.selectedId ? ' is-selected' : '';
    return (
      '<article class="room-card' + selected + '" style="--cat:' + meta.color + '" ' +
      'data-id="' + room.id + '" tabindex="0" role="button">' +
        '<div class="rc-id">' + highlight(room.id + ' 室') + '</div>' +
        '<div class="rc-name">' + highlight(room.name) + '</div>' +
        '<span class="rc-cat">' + meta.label + '</span>' +
      '</article>'
    );
  }

  function renderList() {
    const rooms = getFloorRooms(state.floor).filter(passesFilter);
    floorSections.innerHTML = '';
    emptyHint.hidden = rooms.length !== 0;
    resultCount.textContent = '共 ' + rooms.length + ' 间';

    if (rooms.length) {
      const group = document.createElement('div');
      group.className = 'floor-group';
      group.innerHTML =
        '<div class="floor-label">济事楼 ' + state.floor + ' 楼 · ' + rooms.length + ' 间</div>' +
        '<div class="room-grid">' + rooms.map(roomCardHtml).join('') + '</div>';
      floorSections.appendChild(group);
    }

    floorSections.querySelectorAll('.room-card').forEach((card) => {
      const open = () => selectRoom(card.dataset.id);
      card.addEventListener('click', open);
      card.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); open(); }
      });
    });
  }

  // ============== 渲染：房间详情 ==============
  function renderDetail() {
    const room = ROOM_MAP[state.selectedId];
    if (!room) {
      detailPanel.innerHTML =
        '<div class="detail-placeholder">' +
          '<div class="ph-icon">🏛️</div>' +
          '<p>点击平面图上的任意房间<br>或在搜索框中输入关键字</p>' +
        '</div>';
      return;
    }
    const meta = CATEGORY_META[room.category] || CATEGORY_META.unknown;

    const staffHtml = room.staff.length
      ? '<div class="staff-list">' + room.staff.map((s) => {
          const titlePart = s.title ? '<span class="staff-title">' + highlight(s.title) + '</span>' : '';
          const notePart  = s.note  ? '<div class="staff-note">' + highlight(s.note) + '</div>' : '';
          return (
            '<div class="staff-item">' +
              '<span class="staff-avatar">' + escapeHtml(s.name.charAt(0)) + '</span>' +
              '<div class="staff-info">' +
                '<div class="staff-line">' +
                  '<span class="staff-name">' + highlight(s.name) + '</span>' + titlePart +
                '</div>' +
                notePart +
              '</div>' +
            '</div>'
          );
        }).join('') + '</div>'
      : '<p class="staff-empty">该房间暂无登记人员信息。</p>';

    const tagsHtml = room.tags.map((t) =>
      '<span class="dc-tag" data-tag="' + escapeHtml(t) + '">#' + escapeHtml(t) + '</span>'
    ).join('') || '<span class="staff-empty">—</span>';

    detailPanel.innerHTML =
      '<div class="detail-card">' +
        '<div class="dc-header" style="--cat:' + meta.color + '">' +
          '<div class="dc-id">济事楼 ' + room.floor + ' 楼 · ' + room.id + ' 室</div>' +
          '<div class="dc-name">' + highlight(room.name) + '</div>' +
          '<div class="dc-cat">' + meta.label + '</div>' +
        '</div>' +
        '<div class="dc-body">' +
          '<div class="dc-section"><h4>房间用途</h4><p>' + highlight(room.purpose) + '</p></div>' +
          '<div class="dc-section"><h4>房间人员</h4>' + staffHtml + '</div>' +
          '<div class="dc-section"><h4>开放时间</h4><p>' + escapeHtml(room.hours) + '</p></div>' +
          '<div class="dc-section"><h4>相关标签</h4><div class="dc-tags">' + tagsHtml + '</div></div>' +
        '</div>' +
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
      '<div class="tt-head" style="background:' + meta.color + '">' + room.id + ' · ' + escapeHtml(room.name) + '</div>' +
      '<div class="tt-body">' +
        '<div>' + escapeHtml(meta.label) + '</div>' +
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
    if (x + rect.width > window.innerWidth)  x = e.clientX - rect.width - pad;
    if (y + rect.height > window.innerHeight) y = e.clientY - rect.height - pad;
    tooltip.style.left = x + 'px';
    tooltip.style.top = y + 'px';
  }
  function hideTooltip() { tooltip.hidden = true; }

  // ============== 选择房间 ==============
  function selectRoom(id) {
    state.selectedId = id;
    const room = ROOM_MAP[id];
    if (room && String(room.floor) !== state.floor) {
      state.floor = String(room.floor);
      document.querySelectorAll('.floor-btn').forEach((b) => {
        b.classList.toggle('is-active', b.dataset.floor === state.floor);
      });
    }
    render();
    if (window.innerWidth <= 900) {
      detailPanel.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
  }

  // ============== 统一渲染入口 ==============
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

  // ============== 事件绑定 ==============
  document.querySelectorAll('.floor-btn').forEach((btn) => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.floor-btn').forEach((b) => b.classList.remove('is-active'));
      btn.classList.add('is-active');
      state.floor = btn.dataset.floor;
      render();
    });
  });

  document.querySelectorAll('.view-btn').forEach((btn) => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.view-btn').forEach((b) => b.classList.remove('is-active'));
      btn.classList.add('is-active');
      state.view = btn.dataset.view;
      render();
    });
  });

  searchInput.addEventListener('input', () => {
    state.keyword = searchInput.value.trim().toLowerCase();
    searchClear.classList.toggle('visible', searchInput.value.length > 0);
    // 搜索时若命中房间在另一楼层，自动切换楼层
    if (state.keyword) {
      const hits = ROOM_DATA.filter(passesFilter);
      if (hits.length && !hits.some((r) => String(r.floor) === state.floor)) {
        state.floor = String(hits[0].floor);
        document.querySelectorAll('.floor-btn').forEach((b) => {
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
      e.preventDefault();
      searchInput.focus();
    } else if (e.key === 'Escape' && state.selectedId) {
      state.selectedId = null;
      render();
    }
  });

  // 初始化
  render();
})();
