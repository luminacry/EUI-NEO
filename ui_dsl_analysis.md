# EUI-NEO 声明式 UI DSL 设计分析

## 目标

目标不是继续简化现有 `MainPage.cpp` / `AnimationPage.cpp` 的写法，而是把整套页面书写方式改成接近 QML 的声明式 DSL。

最终页面代码必须满足：

1. `pages` 里只写声明，不写实现细节
2. 页面只出现 `ui.begin()` 之后的链式控件声明
3. 页面里不再出现 `Panel`、`Label`、`PanelState`、`Place(...)`、`Apply(...)`
4. 每个组件都自带默认属性，开发者只改自己关心的部分
5. 所有控件共享一套基础属性 API，例如位置、大小、缩放、透明度、旋转、圆角、渐变、阴影

你要的目标写法应该是这种：

```cpp
ui.begin("main");

ui.sidebar("main.sidebar")
    .width(60, 200)
    .background(0.12f, 0.12f, 0.14f)
    .item("🏠", "Home", [this]{ SwitchView(MainPageView::Home); })
    .item("✨", "Animation", [this]{ SwitchView(MainPageView::Animation); })
    .themeToggle([this]{ ToggleTheme(); })
    .build();

ui.button("home.primary")
    .text("Primary")
    .x(120)
    .y(80)
    .onClick([this]{ progress += 0.1f; })
    .build();

ui.end();
```

页面里不应该再混入任何其他格式。

## 核心原则

### 1. 所有控件都基于一个基础图元

底层应该先统一成一个基础图元节点，例如：

```cpp
class UINode;
```

所有控件都继承或组合这一个节点。

这个基础图元必须天然带有所有通用属性：

- `x`
- `y`
- `width`
- `height`
- `minWidth`
- `minHeight`
- `maxWidth`
- `maxHeight`
- `scale`
- `scaleX`
- `scaleY`
- `rotation`
- `opacity`
- `translateX`
- `translateY`
- `anchor`
- `rounding`
- `color`
- `gradient`
- `borderWidth`
- `borderColor`
- `blur`
- `shadow`
- `visible`
- `enabled`
- `zIndex`

也就是说，不管是按钮、输入框、侧边栏、卡片、标签、下拉框，都先是一个“基础图元”，然后才叠加自己的业务能力。

### 2. 组件只是在基础图元上加默认行为

例如：

- `Button` 本质上是“可点击图元”
- `Label` 本质上是“带文本的图元”
- `Sidebar` 本质上是“纵向容器图元”
- `ComboBox` 本质上是“带展开状态的图元”
- `Slider` 本质上是“带 value 和拖动行为的图元”

所以 `components` 里的每个组件，都应该：

1. 继承统一基础图元能力
2. 带一套内置默认样式
3. 带自己的事件和数据模型 API

## 组件默认属性

你要求的重点是：

- 开发者不写大小，也能有默认大小
- 不写圆角，也有默认圆角
- 不写 hover，也有默认 hover 表现
- 不写颜色变化，也有默认交互反馈

所以 `components` 里每个控件都应该有内置默认值。

### Button 默认值示例

```cpp
Button
```

默认属性：

- 默认宽度 `120`
- 默认高度 `40`
- 默认圆角 `12`
- 默认背景色 `CurrentTheme->surface`
- 默认文字色 `CurrentTheme->text`
- 默认 hover 颜色变化
- 默认 press 颜色变化
- 默认点击动画

页面里只需要写：

```cpp
ui.button("home.primary")
    .text("Primary")
    .build();
```

如果不写：

- 宽高，走默认值
- 圆角，走默认值
- hover，走默认值
- active，走默认值

### Sidebar 默认值示例

默认属性：

- 默认收起宽度 `60`
- 默认展开宽度 `200`
- 默认背景
- 默认内边距
- 默认选中项高亮
- 默认 item hover 动画
- 默认图标和文本排布

页面里只需要写：

```cpp
ui.sidebar("main.sidebar")
    .item("🏠", "Home", []{})
    .item("⚙", "Settings", []{})
    .build();
```

### ComboBox 默认值示例

默认属性：

- 默认宽度 `220`
- 默认高度 `36`
- 默认圆角 `10`
- 默认展开动画
- 默认 hover 高亮
- 默认选中项样式

页面里只要写：

```cpp
ui.combo("home.combo")
    .items({"Apple", "Banana", "Cherry"})
    .build();
```

## 页面层应该长什么样

页面层最终必须像 QML 一样，只负责描述界面结构。

### 目标写法

```cpp
#include "ui/UIContext.h"

void MainPage::Compose(UIContext& ui) {
    ui.begin("main");

    ui.sidebar("main.sidebar")
        .brand("EUI", "NEO")
        .width(60, 200)
        .item("🏠", "Home", [this]{ SwitchView(MainPageView::Home); })
        .item("✨", "Animation", [this]{ SwitchView(MainPageView::Animation); })
        .themeToggle([this]{ ToggleTheme(); })
        .build();

    ui.glassPanel("main.content")
        .fillRemaining()
        .blur(0.08f)
        .shadow(20.0f, 10.0f, Color(0, 0, 0, 0.30f))
        .build();

    ui.button("home.primary")
        .text("Primary")
        .x(120)
        .y(80)
        .onClick([this]{
            progress += 0.1f;
        })
        .build();

    ui.slider("home.slider")
        .x(120)
        .y(140)
        .value(progress)
        .onChange([this](float v){
            progress = v;
        })
        .build();

    ui.combo("home.combo")
        .x(120)
        .y(200)
        .items({"Item 1", "Item 2", "Item 3"})
        .build();

    ui.end();
}
```

## 页面层禁止出现的内容

如果目标真的是“页面里只有 QML 式声明”，那以后 `pages` 里应该禁止出现：

- `Panel panel;`
- `Label label;`
- `Button button;`
- `panel.x = ...`
- `label.y = ...`
- `widget.width = ...`
- `widget.height = ...`
- `Apply(...)`
- `Place(...)`
- `PanelState`
- `RectFrame`
- `PanelStateAnimation`
- `FloatAnimation`
- `UpdateLayout()`
- `ApplyVisual()`
- `DrawSidebar()`
- `DrawHome()`

这些都应该下沉到：

- `ui/`
- `components/`
- `framework/`

而不是继续留在 `pages/`。

## 需要的 DSL 分层

要做到这一点，必须做完整分层。

### 1. `UIContext`

页面入口统一成：

```cpp
ui.begin("page.id");
ui.end();
```

负责：

- 页面上下文
- 组件注册
- 稳定 key
- 构建顺序
- 组件树

### 2. 基础 Builder

所有组件都对应一个 builder：

- `button()`
- `label()`
- `panel()`
- `sidebar()`
- `slider()`
- `combo()`
- `input()`
- `segmented()`
- `card()`
- `column()`
- `row()`
- `grid()`

### 3. 容器布局 Builder

如果页面不想手写坐标，就必须有布局容器：

- `row()`
- `column()`
- `stack()`
- `grid()`
- `anchor()`
- `fill()`
- `centerIn()`

比如：

```cpp
ui.column("home.controls")
    .centerIn("main.content")
    .spacing(16)
    .child(
        ui.button("home.primary")
            .text("Primary")
    )
    .child(
        ui.input("home.input")
            .placeholder("Type something...")
    )
    .build();
```

### 4. 动画 DSL

你前面一直强调动画必须非常直接，所以动画也要是声明式：

```cpp
ui.button("home.primary")
    .text("Primary")
    .hover()
        .background(CurrentTheme->primary)
        .scale(1.03f)
        .duration(0.18f)
    .end()
    .press()
        .scale(0.98f)
        .duration(0.10f)
    .end()
    .build();
```

或者：

```cpp
ui.icon("theme.icon")
    .text("☀")
    .onPressAnimate()
        .rotateTo(28.0f, 0.14f)
        .queueRotateTo(-24.0f, 0.12f)
        .queueRotateTo(0.0f, 0.18f)
    .build();
```

页面里不应该再看到：

- `Bind`
- `PlayTo`
- `Queue`
- `Update`
- `MarkDirty`

## 基础 API 应该统一暴露

所有控件都应该自动拥有这批基础链式 API：

### 几何

- `.x(...)`
- `.y(...)`
- `.position(x, y)`
- `.width(...)`
- `.height(...)`
- `.size(w, h)`
- `.minWidth(...)`
- `.minHeight(...)`
- `.maxWidth(...)`
- `.maxHeight(...)`
- `.anchor(...)`

### 变换

- `.scale(...)`
- `.scaleX(...)`
- `.scaleY(...)`
- `.rotation(...)`
- `.translateX(...)`
- `.translateY(...)`

### 外观

- `.opacity(...)`
- `.background(...)`
- `.gradient(...)`
- `.rounding(...)`
- `.border(...)`
- `.blur(...)`
- `.shadow(...)`

### 状态

- `.visible(...)`
- `.enabled(...)`
- `.zIndex(...)`

### 事件

- `.onClick(...)`
- `.onHover(...)`
- `.onPress(...)`
- `.onRelease(...)`
- `.onChange(...)`

也就是说，组件自己的 API 只负责新增自己的能力。

例如：

- `button().text(...)`
- `combo().items(...)`
- `slider().value(...)`
- `sidebar().item(...)`

除此之外的基础能力，全都来自同一个基础图元。


## 推荐的最终调用风格

### Sidebar

```cpp
ui.sidebar("main.sidebar")
    .brand("EUI", "NEO")
    .width(60, 200)
    .background(CurrentTheme->surface)
    .item("🏠", "Home", [this]{})
    .item("🔍", "Search", [this]{})
    .item("⚙", "Settings", [this]{})
    .build();
```

### Button

```cpp
ui.button("home.primary")
    .text("Primary")
    .position(120, 80)
    .onClick([this]{ progress += 0.1f; })
    .build();
```

### Button 使用默认属性

```cpp
ui.button("dialog.ok")
    .text("OK")
    .build();
```

上面这种写法意味着：

- 不写宽高，用默认宽高
- 不写圆角，用默认圆角
- 不写 hover，用默认 hover
- 不写按压动画，用默认按压动画

### ComboBox

```cpp
ui.combo("home.combo")
    .position(120, 200)
    .items({"Item 1", "Item 2", "Item 3"})
    .build();
```

### Slider

```cpp
ui.slider("home.slider")
    .position(120, 150)
    .value(progress)
    .onChange([this](float v){ progress = v; })
    .build();
```

## 实现顺序建议

不要再继续微调现有页面代码了，应该直接按下面顺序重构：

1. 先建立统一基础图元 `UIPrimitive`
2. 再把通用基础属性全部并入 `UIPrimitive`
3. 再给 `components` 中每个组件加默认样式和默认行为
4. 再实现 `UIContext + Builder`
5. 再实现 `ui.begin() / ui.end()`
6. 再做 `button / sidebar / slider / combo / input / segmented / card`
7. 再做 `row / column / grid / stack`
8. 最后再把 `pages` 彻底改成声明式 DSL

## 结论

结论只有一句：

你要的是一套像 QML 的声明式 UI 系统，不是继续优化当前这种“页面参与实现”的写法。

所以真正的下一步应该是：

- 底层统一成单图元基类
- 所有组件共享基础 API
- 所有组件提供内置默认属性
- 页面层只保留 `ui.begin()` 之后的链式声明

做到这一步之后，`pages` 里才能真正只剩下：

```cpp
ui.begin("main");
ui.sidebar(...).item(...).build();
ui.button(...).text(...).build();
ui.combo(...).items(...).build();
ui.end();
```

这才是你要的最终形态。
