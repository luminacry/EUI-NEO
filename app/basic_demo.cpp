#include "app/DslAppRuntime.h"
#include <algorithm>

int main() {
    EUINEO::DslAppConfig config;
    config.title = "EUI Basic Contact Demo";
    config.width = 960;
    config.height = 640;
    config.pageId = "basic_demo";
    config.fps = 120;

    return EUINEO::RunDslApp(config, [](EUINEO::UIContext& ui, const EUINEO::RectFrame& screen) {
        EUINEO::UseDslLightTheme(EUINEO::Color(0.0f, 0.0f, 0.0f, 1.0f));

        const float cardW = std::min(520.0f, screen.width - 32.0f);
        const float cardH = 240.0f;
        const float cardX = (screen.width - cardW) * 0.5f;
        const float cardY = (screen.height - cardH) * 0.5f;

        ui.panel("basic.card")
            .position(cardX, cardY)
            .size(cardW, cardH)
            .rounding(24.0f)
            .background(EUINEO::Color(0.09f, 0.09f, 0.11f, 1.0f))
            .border(1.0f, EUINEO::Color(0.22f, 0.22f, 0.28f, 1.0f))
            .build();

        ui.label("basic.title")
            .position(cardX + 34.0f, cardY + 72.0f)
            .fontSize(22.0f)
            .color(EUINEO::Color(0.70f, 0.72f, 0.78f, 1.0f))
            .text("Email")
            .build();

        ui.label("basic.email")
            .position(cardX + 34.0f, cardY + 124.0f)
            .fontSize(38.0f)
            .color(EUINEO::Color(0.97f, 0.97f, 0.98f, 1.0f))
            .text("sudoevolve@gmail.com")
            .build();

        ui.button("basic.github.button")
            .position(cardX + cardW - 106.0f, cardY + cardH - 98.0f)
            .size(72.0f, 62.0f)
            .rounding(18.0f)
            .style(EUINEO::ButtonStyle::Primary)
            .icon("\xE2\x86\x97")
            .fontSize(32.0f)
            .onClick([]() {
                EUINEO::OpenDslUrl("https://github.com/sudoevolve");
            })
            .build();
    });
}
