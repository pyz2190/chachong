/* 济事楼导览系统 —— 交互逻辑 */
(function () {
  'use strict';

  // ----- 应用状态 -----
  const state = {
    floor: 'all',          // 'all' | '4' | '5'
    categories: new Set(), // 选中的分类，空表示全部
    keyword: '',           // 搜索关键字（小写）
    selectedId: null
  };

  // ----- DOM 引用 -----
  const $ = (sel) => document.querySelector(sel);
  const searchInput = $('#searchInput');
  const searchClear = $('#searchClear');
  const floorSections = $('#floorSections');
  const detailPanel = $('#detailPanel');
  const categoryFilter = $('#categoryFilter');
  const resultCount = $('#resultCount');
  const emptyHint = $('#emptyHint');

  // ----- 工具函数 -----
  function escapeHtml(str) {
    return String(str).replace(/[&<>"']/g, (c) => ({
      '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
    }[c]));
  }

  // 在文本中高亮关键字
  function highlight(text) {
    const safe = escapeHtml(text);
    if (!state.keyword) return safe;
    const kw = state.keyword.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    return safe.replace(new RegExp('(' + kw + ')', 'gi'), '<mark>$1</mark>');
  }

  // 将一位教师条目展开为可检索的字符串
  function staffText(s) {
    return [s.name, s.title, s.note].filter(Boolean).join(' ');
  }

  // 判断房间是否命中搜索关键字（房间号 / 名称 / 用途 / 人员姓名职称 / 标签）
  function matchesKeyword(room) {
    if (!state.keyword) return true;
    const haystack = [
      room.id, room.name, room.purpose,
      room.staff.map(staffText).join(' '),
      room.tags.join(' '),
      CATEGORY_META[room.category].label
    ].join(' ').toLowerCase();
    return haystack.indexOf(state.keyword) !== -1;
  }

  // 依据当前状态过滤房间
  function getFilteredRooms() {
    return ROOM_DATA.filter((room) => {
      if (state.floor !== 'all' && String(room.floor) !== state.floor) return false;
      if (state.categories.size && !state.categories.has(room.category)) return false;
      return matchesKeyword(room);
    });
  }

  // ----- 渲染：分类筛选条 -----
  function renderCategoryFilter() {
    categoryFilter.innerHTML = '';
    Object.keys(CATEGORY_META).forEach((key) => {
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

  // ----- 渲染：房间总览 -----
  function roomCardHtml(room) {
    const meta = CATEGORY_META[room.category];
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

  function renderOverview() {
    const rooms = getFilteredRooms();
    floorSections.innerHTML = '';
    emptyHint.hidden = rooms.length !== 0;
    resultCount.textContent = '共 ' + rooms.length + ' 间';

    [4, 5].forEach((floor) => {
      const floorRooms = rooms.filter((r) => r.floor === floor);
      if (!floorRooms.length) return;
      const group = document.createElement('div');
      group.className = 'floor-group';
      group.innerHTML =
        '<div class="floor-label">济事楼 ' + floor + ' 楼 · ' + floorRooms.length + ' 间</div>' +
        '<div class="room-grid">' + floorRooms.map(roomCardHtml).join('') + '</div>';
      floorSections.appendChild(group);
    });

    floorSections.querySelectorAll('.room-card').forEach((card) => {
      const open = () => selectRoom(card.dataset.id);
      card.addEventListener('click', open);
      card.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); open(); }
      });
    });
  }

  // ----- 渲染：房间详情 -----
  function renderDetail() {
    const room = ROOM_DATA.find((r) => r.id === state.selectedId);
    if (!room) {
      detailPanel.innerHTML =
        '<div class="detail-placeholder">' +
          '<div class="ph-icon">🏛️</div>' +
          '<p>从左侧选择任意房间<br>即可在此查看详细信息</p>' +
        '</div>';
      return;
    }
    const meta = CATEGORY_META[room.category];

    const staffHtml = room.staff.length
      ? '<div class="staff-list">' + room.staff.map((s) => {
          const titlePart = s.title ? '<span class="staff-title">' + highlight(s.title) + '</span>' : '';
          const notePart  = s.note  ? '<div class="staff-note">' + highlight(s.note) + '</div>' : '';
          return (
            '<div class="staff-item">' +
              '<span class="staff-avatar">' + escapeHtml(s.name.charAt(0)) + '</span>' +
              '<div class="staff-info">' +
                '<div class="staff-line">' +
                  '<span class="staff-name">' + highlight(s.name) + '</span>' +
                  titlePart +
                '</div>' +
                notePart +
              '</div>' +
            '</div>'
          );
        }).join('') + '</div>'
      : '<p class="staff-empty">该房间为公共空间，无固定人员。</p>';

    const tagsHtml = room.tags.map((t) =>
      '<span class="dc-tag" data-tag="' + escapeHtml(t) + '">#' + escapeHtml(t) + '</span>'
    ).join('');

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

    // 点击标签即以该标签作为关键字检索
    detailPanel.querySelectorAll('.dc-tag').forEach((tag) => {
      tag.addEventListener('click', () => {
        searchInput.value = tag.dataset.tag;
        state.keyword = tag.dataset.tag.toLowerCase();
        searchClear.classList.add('visible');
        render();
      });
    });
  }

  // ----- 选择房间（详情就地刷新，不跳转页面） -----
  function selectRoom(id) {
    state.selectedId = id;
    render();
    if (window.innerWidth <= 860) {
      detailPanel.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
  }

  // ----- 统一渲染入口 -----
  function render() {
    renderCategoryFilter();
    renderOverview();
    renderDetail();
  }

  // ----- 事件绑定 -----
  document.querySelectorAll('.floor-btn').forEach((btn) => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.floor-btn').forEach((b) => b.classList.remove('is-active'));
      btn.classList.add('is-active');
      state.floor = btn.dataset.floor;
      render();
    });
  });

  searchInput.addEventListener('input', () => {
    state.keyword = searchInput.value.trim().toLowerCase();
    searchClear.classList.toggle('visible', searchInput.value.length > 0);
    render();
  });

  searchClear.addEventListener('click', () => {
    searchInput.value = '';
    state.keyword = '';
    searchClear.classList.remove('visible');
    searchInput.focus();
    render();
  });

  // 按 / 键快速聚焦搜索框
  document.addEventListener('keydown', (e) => {
    if (e.key === '/' && document.activeElement !== searchInput) {
      e.preventDefault();
      searchInput.focus();
    }
  });

  // ----- 初始化 -----
  render();
})();
