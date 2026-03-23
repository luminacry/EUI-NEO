# EUI-NEO 渲染性能与问题防回归

## 目标

- 防止以后又把普通交互改回高 CPU / GPU 占用
- 防止以后又把渲染链路改回残影、重影、蓝色残留、裁剪错误、黑屏

---

## 当前稳定基线

当前项目的稳定渲染路径是：

- 事件驱动重绘
- `Backdrop` 单独做 layer cache
- 普通非 blur 节点在最终屏幕绘制阶段走节点表面缓存
- blur 节点直接绘制，不走节点表面缓存
- 页面结构稳定时，节点依赖 compose snapshot 复用

关键文件：

- `main.cpp`
- `src/EUINEO.cpp`
- `src/ui/UIContext.cpp`
- `src/ui/UINode.h`

---

## 当前正确渲染链路

### 1. 主循环

当前主循环必须满足：

- 鼠标移动不直接 `RequestRepaint()`
- 鼠标移动只设置 `State.pointerMoved = true`
- 先 `Update()`，状态真的变了再进入绘制
- 没有输入、没有动画、没有状态变化时允许休眠

这条规则是降低 hover、输入、拖动空耗的关键。

### 2. Layer 策略

当前只保留：

- `RenderLayer::Backdrop` 进 FBO
- `CompositeLayers(...)` 只合成 Backdrop

当前不做：

- `Content`
- `Chrome`
- `Popup`

整层 FBO 合成。

原因：

- 之前把所有 layer 都做离屏缓存和再合成，已经验证会带来残影、蓝色残留、黑屏、投影错位、切页异常

### 3. 节点缓存策略

当前 `UIContext::draw()` 的稳定规则：

- 非 Backdrop 节点参与正常屏幕绘制
- 非 blur 节点走 `Renderer::DrawCachedSurface(...)`
- blur 节点直接绘制

重点：

- 节点缓存只用于最终屏幕绘制阶段
- 不要把节点缓存重新嵌进 layer FBO 绘制链

### 4. Backdrop 失效规则

只有真的影响背景玻璃、背景装饰、主题背景时，才允许：

```cpp
Renderer::InvalidateLayer(RenderLayer::Backdrop);
```

普通前景控件变化不应该顺手把 Backdrop 一起打脏。

---

## 明确禁止回退的路线

下面这些路线不要再回来了：

- 脏区矩形收集
- 脏区合并 / 拆分 / 裁剪
- 局部清屏
- 局部回放
- 遮挡传播
- “A 变了所以推导 B/C 要重绘”的矩形链
- 所有 layer 都走整层 FBO 合成
- 鼠标移动回调里直接 `RequestRepaint()`
- 为了“优化”把普通节点缓存塞回 layer 绘制阶段

这些路线在这个项目里已经验证过会带来：

- 残影
- 重影
- 蓝色残留
- 黑屏
- 错误裁剪
- 复杂度快速失控
- 新增组件越来越难维护

---

## 以后写页面必须守的规则

### 页面规则

- 一个页面一个 `.h`
- 页面只负责 `Compose(...)` 和状态组织
- 节点 key 必须稳定
- 不要在 compose 里生成随机 key
- 页面结构没变时，不要主动破坏 compose 复用

### 状态规则

- 只有真实状态变化才改状态
- 只有真实视觉变化才 `requestVisualRepaint()`
- 不要因为临时 hover 去改页面级 `stateVersion_`
- 不要把小交互升级成整页 recompose

### Backdrop 规则

- 只有背景玻璃 / 背景装饰 / 主题背景变了才打脏 Backdrop
- 前景控件颜色、hover、输入、滑动，不要随手 `InvalidateBackdrop()`

---

## 以后写组件必须守的规则

### 1. 组件必须自己管 `paintBounds()`

如果组件存在下面这些情况，必须自己覆写 `paintBounds()`：

- 阴影
- 模糊
- 外扩高亮
- 位移
- 缩放
- 旋转
- 弹出列表
- 滑块圆点超出本体

否则就会出现：

- 绘制被裁
- 滑块圆点被切
- 下拉框被切
- 背景装饰被切
- 缓存范围错误
- 无意义的大面积重录制

优先使用 `UINode` 里的 helper：

- `measurePrimitivePaintBounds()`
- `expandPrimitivePaintBounds(...)`
- `measurePaintBounds(...)`
- `unionPaintBounds(...)`
- `clipPaintBounds(...)`

### 2. 连续更新必须自动停止

`wantsContinuousUpdate()` 只能在下面这些情况返回 `true`：

- 动画没结束
- 拖动没结束
- 滚动条脉冲效果没结束
- 输入光标闪烁还在运行

不允许：

- 页面静止了还一直返回 `true`
- 组件什么都没动还强制续帧

### 3. 视觉变化和结构变化要分开

组件内部有两类变化：

- 只改视觉：`requestVisualRepaint()`
- 影响结构 / clip / popup 呈现关系：`requestComposeRebuild()`

不要把两类事情混用。

### 4. blur 组件默认当成昂贵节点

blur 节点当前不走普通节点表面缓存。

所以：

- blur 容器要少
- blur 面积要小
- 不要把普通按钮、输入框、列表项默认做成 blur

---

## 防止残影和图层错误的硬规则

### 1. `BeginFrame()` 先清 scissor 影响

`Renderer::BeginFrame()` 当前必须先：

```cpp
glDisable(GL_SCISSOR_TEST);
```

不要删。

这条是防止：

- 局部残留
- 截断错位
- 黑屏

的底线之一。

### 2. `CompositeLayers(...)` 只负责 Backdrop

不要再改成把：

- `Content`
- `Chrome`
- `Popup`

一起走 layer composite。

### 3. 节点缓存不要重新嵌进 layer draw

当前稳定做法：

- `main.cpp` 里 layer draw 只画 Backdrop
- 普通节点缓存只发生在 `UIContext::draw()`

不要再把 `DrawCachedSurface(...)` 放进 `draw(RenderLayer layer)` 的普通前景链路里。

### 4. popup 和滚动必须走框架能力

不要手搓一套局部裁剪和偏移。

当前正确能力是：

- `popup(...)`
- `scrollArea(...)`
- `clipStack`
- `offsetStack`

如果绕开这些重新自己写，很容易重新出现：

- popup 被裁剪
- 滚动内容错位
- 局部重影

---

## 出现高占用时的排查顺序

如果以后又出现 hover、输入、拖动时 GPU / CPU 偏高，按下面顺序查：

1. 有没有把鼠标移动回调改回直接 `RequestRepaint()`
2. 有没有组件长期返回 `wantsContinuousUpdate() == true`
3. 有没有把普通节点缓存关掉
4. 有没有又把所有 layer 改成 FBO 合成
5. 有没有把 blur 用到普通交互控件
6. 有没有把小交互改成整页 `stateVersion_++`
7. 有没有节点 key 不稳定导致缓存失效
8. 有没有组件 `paintBounds()` 过大导致缓存成本虚高

不要第一反应又回到脏区系统。

---

## 出现残影 / 重影 / 蓝色残留时的排查顺序

1. 有没有重新引入脏区 / 局部回放 / 遮挡传播
2. `Renderer::CompositeLayers(...)` 是否又改回多 layer 合成
3. `Renderer::BeginFrame()` 里是否还保留 `glDisable(GL_SCISSOR_TEST)`
4. 是否把节点缓存嵌回了 layer 绘制链
5. 是否某个节点 `paintBounds()` 太小，导致缓存内容被裁
6. 是否 popup / scroll 绕开了统一 clip + offset 体系
7. 是否 backdrop 失效链漏掉，导致旧背景被复用

---

## 出现 popup / 滑块圆点 / 装饰被裁时的排查顺序

1. 节点是否覆写了正确的 `paintBounds()`
2. popup 是否使用 popup 呈现规则
3. 是否通过 `scrollArea(...)` / `popup(...)` 进入框架 clip + offset 流程

---

## 出现黑屏或内容不显示时的排查顺序

1. `CompositeLayers(...)` 是否仍然只合成 Backdrop
2. `BeginFrame()` 的投影边界是否被改坏
3. 是否在错误的 layer / custom surface 状态里直接复合缓存纹理
4. 是否删掉了通用 frame 开始时的 scissor 清理

---

## 出现主题切换后局部不刷新时的排查顺序

1. 是否调用了 `ui_.markAllNodesDirty()`
2. 是否调用了 `Renderer::InvalidateAll()`
3. 如果只影响玻璃背景，是否至少调用了 `Renderer::InvalidateLayer(RenderLayer::Backdrop)`

---

## 以后改渲染链路前必须过的检查

在你改下面这些地方前，先看这份文档：

- `main.cpp`
- `src/EUINEO.cpp`
- `src/ui/UIContext.cpp`
- `src/ui/UINode.h`

每次改完至少自查：

- hover 按钮是否还低占用
- 输入框输入是否还低占用
- 下拉框打开是否还低占用
- 滑条拖动是否还低占用
- 切页是否无残影
- sidebar 是否无蓝色残留
- popup / slider handle / blur 装饰是否无裁剪

---

## 一句话结论

EUI-NEO 当前稳定路线不是脏区系统，而是：

- 事件驱动重绘
- Backdrop 单层缓存
- 普通节点屏幕阶段表面缓存
- blur 直接绘制
- 稳定 key
- 精确 `paintBounds()`
- 只在真实变化时续帧

以后谁再把它改回“局部矩形推导 + 多 layer 乱合成”，谁就很大概率把高占用和残影问题重新带回来。
