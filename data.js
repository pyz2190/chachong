/*
 * 济事楼 4-5 层房间数据
 * 注：以下为课程作业演示用示例数据，房间用途与人员均为虚构，便于展示导览与检索功能。
 * 房间分类 category 取值：office(办公) / teaching(教学) / meeting(会议) / research(研究) / service(服务)
 */
const ROOM_DATA = [
  /* ===================== 4 楼 ===================== */
  {
    id: '401', floor: 4, name: '学工办公室', category: 'service',
    purpose: '负责本科生日常事务管理，包括奖助学金评定、心理咨询预约、就业指导与学生活动审批。',
    staff: ['葛蕾（辅导员）', '李明（辅导员）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['学工办', '辅导员', '奖学金', '心理咨询']
  },
  {
    id: '402', floor: 4, name: '团委办公室', category: 'service',
    purpose: '组织开展共青团工作、社团管理、志愿服务与校园文化活动。',
    staff: ['王芳（团委书记）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['团委', '社团', '志愿服务']
  },
  {
    id: '403', floor: 4, name: '院党委办公室', category: 'office',
    purpose: '承担学院党务工作、组织发展与教职工政治学习的协调安排。',
    staff: ['周建国（党委秘书）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['党委', '党务']
  },
  {
    id: '404', floor: 4, name: '第一会议室', category: 'meeting',
    purpose: '可容纳 30 人的中型会议室，用于院务会议、答辩与小型讲座。',
    staff: [],
    hours: '需提前预约',
    tags: ['会议室', '答辩', '讲座']
  },
  {
    id: '405', floor: 4, name: '教师休息室', category: 'service',
    purpose: '供授课教师课间休息与备课，配有茶水与简易办公设施。',
    staff: [],
    hours: '全天开放',
    tags: ['休息室', '备课']
  },
  {
    id: '406', floor: 4, name: '本科教务办公室', category: 'service',
    purpose: '负责本科教学排课、成绩管理、选课答疑与毕业资格审核。',
    staff: ['张伟（教务员）'],
    hours: '周一至周五 8:30 - 16:30',
    tags: ['教务', '排课', '成绩', '选课']
  },
  {
    id: '407', floor: 4, name: '多媒体教室 A', category: 'teaching',
    purpose: '配备投影与音响系统的多媒体教室，可容纳 80 人，用于专业课授课。',
    staff: [],
    hours: '按课表使用',
    tags: ['教室', '多媒体', '授课']
  },
  {
    id: '408', floor: 4, name: '多媒体教室 B', category: 'teaching',
    purpose: '配备智慧黑板的多媒体教室，可容纳 60 人，支持小班研讨教学。',
    staff: [],
    hours: '按课表使用',
    tags: ['教室', '多媒体', '研讨']
  },
  {
    id: '409', floor: 4, name: '案例研讨室', category: 'teaching',
    purpose: '环形桌椅布局的研讨室，适合案例教学与小组讨论。',
    staff: [],
    hours: '需提前预约',
    tags: ['研讨室', '案例教学', '小组讨论']
  },
  {
    id: '410', floor: 4, name: '资料档案室', category: 'service',
    purpose: '存放学院教学档案、专业图书与历年试卷资料，提供借阅服务。',
    staff: ['孙丽（资料员）'],
    hours: '周一至周五 9:00 - 16:00',
    tags: ['档案', '资料', '借阅']
  },
  {
    id: '411', floor: 4, name: '教师办公室（一）', category: 'office',
    purpose: '专业课教师集中办公区域，提供课后答疑与学生约谈。',
    staff: ['赵敏（讲师）', '钱进（副教授）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['教师办公室', '答疑']
  },
  {
    id: '412', floor: 4, name: '教师办公室（二）', category: 'office',
    purpose: '专业课教师集中办公区域，提供课后答疑与学生约谈。',
    staff: ['吴桐（讲师）', '郑浩（讲师）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['教师办公室', '答疑']
  },

  /* ===================== 5 楼 ===================== */
  {
    id: '501', floor: 5, name: '院长办公室', category: 'office',
    purpose: '学院院长办公场所，统筹学院发展规划与重大事务决策。',
    staff: ['陈强（院长）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['院长', '行政']
  },
  {
    id: '502', floor: 5, name: '副院长办公室', category: 'office',
    purpose: '分管教学与科研的副院长办公场所。',
    staff: ['林雪（副院长）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['副院长', '行政']
  },
  {
    id: '503', floor: 5, name: '学术报告厅', category: 'meeting',
    purpose: '可容纳 150 人的报告厅，用于学术讲座、论坛与大型答辩。',
    staff: [],
    hours: '需提前预约',
    tags: ['报告厅', '讲座', '论坛']
  },
  {
    id: '504', floor: 5, name: '研究生教务办公室', category: 'service',
    purpose: '负责研究生培养方案管理、开题答辩安排与学位申请审核。',
    staff: ['刘洋（研究生秘书）'],
    hours: '周一至周五 8:30 - 16:30',
    tags: ['研究生', '教务', '学位', '答辩']
  },
  {
    id: '505', floor: 5, name: '财务与综合办公室', category: 'office',
    purpose: '处理学院财务报销、资产管理与日常综合事务。',
    staff: ['黄静（财务）'],
    hours: '周一至周五 9:00 - 16:00',
    tags: ['财务', '报销', '资产']
  },
  {
    id: '506', floor: 5, name: '教授工作室（一）', category: 'research',
    purpose: '资深教授科研工作室，承担课题研究与研究生指导。',
    staff: ['马远（教授）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['教授', '科研', '导师']
  },
  {
    id: '507', floor: 5, name: '教授工作室（二）', category: 'research',
    purpose: '资深教授科研工作室，承担课题研究与研究生指导。',
    staff: ['何苗（教授）'],
    hours: '周一至周五 8:30 - 17:00',
    tags: ['教授', '科研', '导师']
  },
  {
    id: '508', floor: 5, name: '博士生工作室', category: 'research',
    purpose: '博士研究生集中科研办公区域，配备独立工位与讨论角。',
    staff: [],
    hours: '全天开放（凭卡进入）',
    tags: ['博士生', '科研', '工位']
  },
  {
    id: '509', floor: 5, name: '科研实验室', category: 'research',
    purpose: '行为与数据实验室，开展问卷实验、眼动与可用性测试研究。',
    staff: ['徐航（实验室管理员）'],
    hours: '需提前预约',
    tags: ['实验室', '科研', '可用性测试']
  },
  {
    id: '510', floor: 5, name: '创新创业实践室', category: 'teaching',
    purpose: '支持学生创新创业项目孵化，提供路演与团队协作空间。',
    staff: [],
    hours: '周一至周五 9:00 - 21:00',
    tags: ['创新创业', '孵化', '路演']
  },
  {
    id: '511', floor: 5, name: '第二会议室', category: 'meeting',
    purpose: '可容纳 20 人的小型会议室，用于课题组会议与师生座谈。',
    staff: [],
    hours: '需提前预约',
    tags: ['会议室', '课题组']
  },
  {
    id: '512', floor: 5, name: '接待室', category: 'service',
    purpose: '用于接待来访嘉宾、校企合作洽谈与对外交流。',
    staff: [],
    hours: '需提前预约',
    tags: ['接待', '校企合作']
  }
];

const CATEGORY_META = {
  office:   { label: '行政办公', color: '#3b6ea5' },
  teaching: { label: '教学空间', color: '#2e8b57' },
  meeting:  { label: '会议研讨', color: '#b8860b' },
  research: { label: '科研空间', color: '#8a5cb0' },
  service:  { label: '服务窗口', color: '#c0563b' }
};
