/*
 * 济事楼 4-5 层房间与人员数据
 * 数据来源：用户上传的 staff.ts（同济大学计算机科学与技术学院 / 济事楼实地信息）
 * 房间分类 category 取值：office(办公) / teaching(教学) / meeting(会议) / research(研究) / service(服务)
 * 每位教师条目结构：{ name, title, note }
 *   - name : 姓名
 *   - title: 职称（教授 / 副教授 / 实验中心 等）
 *   - note : 所属位置或职责（如 "418L 教师工作室"、"研究方向" 等）
 */

const ROOM_DATA = [
  /* ===================== 4 楼 ===================== */
  {
    id: '407', floor: 4, name: '研究生工作室', category: 'research',
    purpose: '研究生科研工作室，承担课题组日常科研任务。',
    hours: '凭卡进入',
    staff: [
      { name: '赵生捷', title: '教授', note: '课题组负责人' },
      { name: '严海洲', title: '实验中心', note: '安全负责人' }
    ],
    tags: ['研究生工作室', '课题组', '科研']
  },
  {
    id: '408', floor: 4, name: '教师工作室（408L / 408R）', category: 'office',
    purpose: '专任教师集中办公区域，分为 408L 与 408R 两间，承担授课、答疑与科研指导。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '史扬',   title: '教授',                note: '408L 教师工作室' },
      { name: '朱宏明', title: '副教授 / 院务助理',    note: '408L 教师工作室' },
      { name: '赵钦佩', title: '副教授',              note: '408L 教师工作室' },
      { name: '唐伟',   title: '预聘助理教授',        note: '408R 教师工作室' },
      { name: '程颖',   title: '助理教授',            note: '408R · 空间智能 / 多媒体分析' },
      { name: '汪昱',   title: '副教授',              note: '408R 教师工作室' },
      { name: '黄凯锋', title: '预聘助理教授',        note: '408R 教师工作室' },
      { name: '李文浩', title: '预聘助理教授',        note: '408R 教师工作室' },
      { name: '高嘉尧', title: '博士后 / 研究助理',    note: '408R 教师工作室' }
    ],
    tags: ['教师工作室', '院务助理', '空间智能', '多媒体分析']
  },
  {
    id: '409', floor: 4, name: '实验室 / 研究生工作室（409L / 409R）', category: 'research',
    purpose: '教育部工程研究中心实验室与研究生工作室。',
    hours: '凭卡进入',
    staff: [
      { name: '叶晨',   title: '实验中心', note: '409L / 409R 平台负责人' },
      { name: '罗怡桂', title: '副教授',   note: '409R 实验室负责人' },
      { name: '谢尹',   title: '实验室安全负责人', note: '409R 实验室安全负责人' }
    ],
    tags: ['教育部工程研究中心', '实验室', '研究生工作室', '平台负责人', '安全负责人']
  },
  {
    id: '410', floor: 4, name: '教师工作室', category: 'office',
    purpose: '专任教师集中办公区域。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '王冬青', title: '讲师',          note: '教师工作室' },
      { name: '李江峰', title: '副教授',        note: '区块链 / 隐私数据智能分析 / 视频情感搜索' },
      { name: '夏波涌', title: '讲师',          note: '教师工作室' },
      { name: '张颖',   title: '副教授',        note: '深度学习 / 智能计算系统' },
      { name: '熊明亮', title: '预聘助理教授',  note: '教师工作室' }
    ],
    tags: ['教师工作室', '区块链', '隐私计算', '深度学习', '智能计算系统']
  },
  {
    id: '412', floor: 4, name: '教师工作室', category: 'office',
    purpose: '专任教师集中办公区域。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '刘岩',   title: '副教授',              note: '教师工作室' },
      { name: '张惠娟', title: '副教授',              note: '教师工作室' },
      { name: '孙萍',   title: '副教授 / 实验中心',    note: '教师工作室' },
      { name: '罗怡桂', title: '副教授',              note: '教师工作室' }
    ],
    tags: ['教师工作室', '实验中心']
  },
  {
    id: '414', floor: 4, name: '教师工作室', category: 'office',
    purpose: '专任教师集中办公区域。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '肖国宝', title: '教授', note: '教师工作室' },
      { name: '杜庆峰', title: '教授', note: '教师工作室' },
      { name: '张苗苗', title: '教授', note: '教师工作室' }
    ],
    tags: ['教师工作室']
  },
  {
    id: '416', floor: 4, name: '多媒体教学机房', category: 'teaching',
    purpose: '面向本科与研究生课程的多媒体教学机房，配备成套实验环境。',
    hours: '按课表使用',
    staff: [
      { name: '杨旻',   title: '资产管理', note: '多媒体教学机房课题组负责人' },
      { name: '严海洲', title: '实验中心', note: '多媒体教学机房安全负责人' }
    ],
    tags: ['多媒体教学机房', '机房', '实验环境']
  },
  {
    id: '418', floor: 4, name: '教师工作室 / 研究生工作室（418L / 418R）', category: 'research',
    purpose: '418L 为教师工作室，418R 为研究生工作室。',
    hours: '凭卡进入',
    staff: [
      { name: '张林',   title: '教授', note: '418L 教师工作室 · 计算机视觉 / 机器视觉 / 机器学习' },
      { name: '刘琴',   title: '教授', note: '418L 教师工作室' },
      { name: '袁时金', title: '教授', note: '418R 研究生工作室 · 课题组与安全负责人' }
    ],
    tags: ['教师工作室', '研究生工作室', '计算机视觉', '机器学习']
  },
  {
    id: '419', floor: 4, name: '计算机系统实验室', category: 'research',
    purpose: '面向计算机系统方向的科研与教学实验室。',
    hours: '凭卡进入',
    staff: [
      { name: '张晶',   title: '实验中心', note: '计算机系统实验室课题组负责人' },
      { name: '严海洲', title: '实验中心', note: '计算机系统实验室安全负责人' }
    ],
    tags: ['计算机系统实验室', '实验中心', '安全负责人']
  },
  {
    id: '428', floor: 4, name: '服务器机房', category: 'research',
    purpose: '学院服务器机房，承载科研与教学计算服务。',
    hours: '凭卡进入（仅限管理人员）',
    staff: [
      { name: '杨旻',   title: '资产管理', note: '服务器机房课题组负责人' },
      { name: '严海洲', title: '实验中心', note: '服务器机房安全负责人' }
    ],
    tags: ['服务器机房', '机房', '安全负责人']
  },
  {
    id: '430', floor: 4, name: '多媒体教学机房', category: 'teaching',
    purpose: '面向本科与研究生课程的多媒体教学机房。',
    hours: '按课表使用',
    staff: [
      { name: '杨旻',   title: '资产管理', note: '多媒体教学机房课题组负责人' },
      { name: '严海洲', title: '实验中心', note: '多媒体教学机房安全负责人' }
    ],
    tags: ['多媒体教学机房', '机房']
  },
  {
    id: '440', floor: 4, name: '综合事务管理中心', category: 'service',
    purpose: '负责学院综合事务、人事人才、财务、资产与空间管理等行政服务。',
    hours: '周一至周五 8:30 - 16:30',
    staff: [
      { name: '张砚秋', title: '主任',     note: '综合事务管理中心' },
      { name: '闫鹏',   title: '人事人才', note: '综合事务管理中心' },
      { name: '钱银飞', title: '财务',     note: '综合事务管理中心' },
      { name: '陈梁',   title: '空间管理', note: '综合事务管理中心' },
      { name: '杨旻',   title: '资产管理', note: '综合事务管理中心' },
      { name: '张曦月', title: '综合',     note: '综合事务管理中心' }
    ],
    tags: ['综合事务', '人事人才', '财务', '空间管理', '资产管理']
  },
  {
    id: '442', floor: 4, name: '教学管理中心', category: 'service',
    purpose: '本科与研究生教学事务管理，包括排课、教务、学位与培养方案。',
    hours: '周一至周五 8:30 - 16:30',
    staff: [
      { name: '杨洪念', title: '研究生教学主管',   note: '教学管理中心' },
      { name: '杨丹',   title: '研究生教学副主管', note: '教学管理中心' },
      { name: '姚仕仪', title: '研究生教务',       note: '教学管理中心' },
      { name: '王彩霞', title: '本科教学主管',     note: '教学管理中心' },
      { name: '刘梦露', title: '本科教务',         note: '教学管理中心' },
      { name: '李慧敏', title: '本科教务',         note: '教学管理中心' },
      { name: '万亚文', title: '研究生教务',       note: '教学管理中心' }
    ],
    tags: ['教学管理中心', '本科教务', '研究生教务', '教学主管']
  },
  {
    id: '443', floor: 4, name: '综合事务管理中心', category: 'service',
    purpose: '综合事务管理中心人事与高层次人才事务。',
    hours: '周一至周五 8:30 - 16:30',
    staff: [
      { name: '闫志威', title: '副主任',         note: '综合事务管理中心' },
      { name: '张晓雅', title: '人事人才',       note: '综合事务管理中心' },
      { name: '陈妍妍', title: '高层次人才专员', note: '综合事务管理中心' }
    ],
    tags: ['综合事务', '人事人才', '高层次人才']
  },
  {
    id: '444', floor: 4, name: '科研管理中心 / 学科建设与国际合作中心', category: 'service',
    purpose: '科研项目管理、博士后管理、学科建设与国际合作、留学生教务。',
    hours: '周一至周五 8:30 - 16:30',
    staff: [
      { name: '俞晓静', title: '博士后、科研管理',           note: '科研管理中心' },
      { name: '郭玉臣', title: '科研管理主管',               note: '科研管理中心' },
      { name: '杜博闻', title: '学科建设与国际合作主管',     note: '学科建设与国际合作中心' },
      { name: '刘鹏飞', title: '学科建设与国际合作',         note: '学科建设与国际合作中心' },
      { name: '范险若', title: '留学生教务',                 note: '教学管理中心 / 留学生教务' },
      { name: '周琳',   title: '本科教务',                   note: '教学管理中心 / 本科教务' }
    ],
    tags: ['科研管理', '博士后', '学科建设', '国际合作', '留学生教务']
  },
  {
    id: '446', floor: 4, name: '学生思政工作中心', category: 'service',
    purpose: '负责学生思想政治教育、辅导员日常工作、学生事务与心理咨询。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '克热木', title: '主任',   note: '学生思政工作中心' },
      { name: '葛蕾',   title: '辅导员', note: '学生思政工作中心 · 学生工作' },
      { name: '焦嘉欣', title: '辅导员', note: '学生思政工作中心 · 学生工作' },
      { name: '钟梦莹', title: '辅导员', note: '学生思政工作中心 · 学生工作' },
      { name: '王晓文', title: '辅导员', note: '学生思政工作中心 · 学生工作' }
    ],
    tags: ['学工办', '学生思政', '辅导员', '学生工作']
  },
  {
    id: '448', floor: 4, name: '副书记办公室 / 院务助理办公室（448-1 / 448-2 / 448-3）', category: 'office',
    purpose: '党委副书记与院务助理办公室，分管纪检、统战、学生思政、学科建设等。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '陈荣',   title: '党委副书记 / 纪委书记',
        note: '448-1 · 纪检监察 / 统战工会 / 教师思政' },
      { name: '吴晓培', title: '党委副书记',
        note: '448-2 · 学生思想政治教育 / 宣传共青团 / 关心下一代' },
      { name: '宋井宽', title: '教授 / 院务助理',
        note: '448-3 · 学科建设 / 研究生招生 / 人才与国际合作' }
    ],
    tags: ['副书记办公室', '院务助理', '纪检监察', '统战工会', '学生思政', '学科建设']
  },
  {
    id: '450', floor: 4, name: '院长 / 党委书记办公室（450L / 450R）', category: 'office',
    purpose: '450L 为党委书记办公室，450R 为院长办公室。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '申恒涛', title: '院长 / 教授',
        note: '450R 院长办公室 · 多媒体搜索 / 计算机视觉 / 人工智能 / 大数据管理' },
      { name: '熊岚',   title: '党委书记',
        note: '450L 党委书记办公室 · 学院党委 / 组织人事 / 安全稳定 / 财务工作' }
    ],
    tags: ['院长办公室', '党委书记', '人工智能', '多媒体搜索']
  },
  {
    id: '451', floor: 4, name: '副院长办公室（451-1 / 451-2 / 451-3）', category: 'office',
    purpose: '三位副院长办公室，分管行政、科研、教学。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '王成',   title: '教授 / 副院长',
        note: '451-1 · 日常行政 / 资产采招 / 安全生产 / 信息化建设' },
      { name: '何良华', title: '教授 / 副院长',
        note: '451-2 · 科研工作 / 实验室建设与管理 / 产学研' },
      { name: '卫志华', title: '教授 / 副院长',
        note: '451-3 · 本科招生与教学 / 研究生教学 / 人才培养' }
    ],
    tags: ['副院长办公室', '科研', '本科招生', '研究生教学', '人才培养']
  },
  {
    id: '456', floor: 4, name: '党委办公室', category: 'office',
    purpose: '学院党务工作与组织事务办公场所。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '周微微', title: '组织员',       note: '党委办公室 · 党务工作' },
      { name: '陆凤兰', title: '组织员',       note: '党委办公室 · 党务工作' },
      { name: '赵清理', title: '党务工作人员', note: '党委办公室 · 党务工作' }
    ],
    tags: ['党委办公室', '党务工作', '组织员']
  },

  /* ===================== 5 楼 ===================== */
  {
    id: '507', floor: 5, name: '教师工作室', category: 'office',
    purpose: '专任教师集中办公区域。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '陈伟超', title: '预聘助理教授',
        note: '无线通信网络 / 人工智能应用 / 图像视频传输' },
      { name: '王洁',   title: '副教授',
        note: '边缘计算 / 联邦学习 / 网络系统' },
      { name: '曾进',   title: '预聘助理教授', note: '教师工作室' },
      { name: '朱亚萍', title: '预聘副教授',
        note: '无线定位 / 隐私计算 / 多模态数据融合' },
      { name: '杜博闻', title: '预聘助理教授', note: '教师工作室' },
      { name: '韩丰夏', title: '副教授',
        note: '通信感知一体化 / 物联网 / 智慧城市' },
      { name: '李冰',   title: '副教授',
        note: '无人机 / 低空经济 / 智慧交通' }
    ],
    tags: ['教师工作室', '无线通信', '边缘计算', '联邦学习', '隐私计算', '物联网', '低空经济']
  },
  {
    id: '508', floor: 5, name: '研究生工作室', category: 'research',
    purpose: '研究生工作室，承担课题组科研任务。',
    hours: '凭卡进入',
    staff: [
      { name: '叶晨',   title: '实验中心',       note: '研究生工作室平台负责人' },
      { name: '尹长青', title: '教授',           note: '研究生工作室实验室负责人' },
      { name: '陈韩悦', title: '实验室安全负责人', note: '研究生工作室安全负责人' }
    ],
    tags: ['研究生工作室', '平台负责人', '实验室负责人', '安全负责人']
  },
  {
    id: '509', floor: 5, name: '教师工作室（509R）', category: 'office',
    purpose: '专任教师集中办公区域。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '尹长青', title: '教授',     note: '509R 教师工作室' },
      { name: '沈莹',   title: '教授',
        note: '509R · 用户交互技术 / 语音信号处理 / 自然语言处理 / 生物信息学' },
      { name: '金博',   title: '副教授',
        note: '509R · 机器学习 / 多智能体强化学习' }
    ],
    tags: ['教师工作室', '自然语言处理', '语音信号', '机器学习', '多智能体强化学习']
  },
  {
    id: '510', floor: 5, name: '教师工作室', category: 'office',
    purpose: '专任教师工作室。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '赵生捷', title: '教授', note: '教师工作室' }
    ],
    tags: ['教师工作室']
  },
  {
    id: '511', floor: 5, name: '研究生工作室', category: 'research',
    purpose: '研究生科研工作室。',
    hours: '凭卡进入',
    staff: [
      { name: '叶晨',   title: '实验中心',       note: '研究生工作室平台负责人' },
      { name: '罗怡桂', title: '副教授',         note: '研究生工作室实验室负责人' },
      { name: '谢尹',   title: '实验室安全负责人', note: '研究生工作室安全负责人' }
    ],
    tags: ['研究生工作室', '平台负责人', '实验室负责人']
  },
  {
    id: '514', floor: 5, name: '教师工作室', category: 'office',
    purpose: '专任教师集中办公区域。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '黄杰',   title: '实验中心',         note: '教师工作室' },
      { name: '高珍',   title: '副教授',
        note: '自动驾驶 / 智能交通系统 / 交通安全分析' },
      { name: '冯巾松', title: '讲师',
        note: '计算机应用 / 大型机开发技术' },
      { name: '范鸿飞', title: '副教授 / 院务助理',
        note: '软件工程 / 人机交互 / 协同计算' },
      { name: '唐剑锋', title: '讲师',
        note: '机器学习 / 大型数据库系统管理 / 离散数学' },
      { name: '邓浩',   title: '预聘副教授',
        note: '用户交互设计 / 机器学习与动力学模型 / 城市数据智能' }
    ],
    tags: ['教师工作室', '自动驾驶', '智能交通', '软件工程', '人机交互', '机器学习']
  },
  {
    id: '517', floor: 5, name: '主机服务器机房', category: 'research',
    purpose: '学院主机服务器机房，承担大规模科研与教学计算服务。',
    hours: '凭卡进入（仅限管理人员）',
    staff: [
      { name: '杨旻', title: '资产管理', note: '主机服务器机房负责人 / 安全负责人' }
    ],
    tags: ['主机服务器机房', '机房', '实验室安全']
  },
  {
    id: '518', floor: 5, name: '教师工作室（518L / 518R）', category: 'office',
    purpose: '专任教师集中办公区域，分为 518L 与 518R 两间。',
    hours: '周一至周五 8:30 - 17:00',
    staff: [
      { name: '徐行',   title: '教授',
        note: '518L · 人工智能 / 多媒体 / 计算机视觉' },
      { name: '曹晓锋', title: '长聘副教授',
        note: '518L · 机器学习理论 / 非欧氏几何 / 物理人工智能' },
      { name: '王轩瀚', title: '研究员',
        note: '518L · 视觉智能 / 多模态具身智能' },
      { name: '朱磊',   title: '研究员 / 院务助理',
        note: '518R · 多模态学习 / 大模型与算法' },
      { name: '张鹏飞', title: '教授',     note: '518R 教师工作室' },
      { name: '张奇',   title: '长聘副教授',
        note: '518R · 人工智能 / 数据科学 / 脑认知分析' }
    ],
    tags: ['教师工作室', '人工智能', '计算机视觉', '多模态', '大模型', '机器学习理论']
  },
  {
    id: '519', floor: 5, name: '实验机房（ACM-ICPC / 贝宝实验室）', category: 'research',
    purpose: 'ACM-ICPC 训练中心、eBay 国家级工程实践教育中心与贝宝实验室所在机房。',
    hours: '按训练 / 项目安排',
    staff: [
      { name: '杨旻', title: '资产管理',
        note: '实验机房负责人 / 安全负责人 · ACM-ICPC 训练中心 / eBay 工程实践教育中心 / 贝宝实验室' }
    ],
    tags: ['实验机房', 'ACM-ICPC', 'eBay', '贝宝实验室', '工程实践教育中心']
  }
];

const CATEGORY_META = {
  office:   { label: '行政办公', color: '#3b6ea5' },
  teaching: { label: '教学空间', color: '#2e8b57' },
  meeting:  { label: '会议研讨', color: '#b8860b' },
  research: { label: '科研空间', color: '#8a5cb0' },
  service:  { label: '服务窗口', color: '#c0563b' }
};
