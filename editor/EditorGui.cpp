#include "StdAfx.h"

#include "Engine.h"
#include "EditorGui.h"

EditorGui::EditorGui() {
}

void EditorGui::Start() {
}

void EditorGui::Init() {
    m_buttons.clear();
    const Size default_size = Size(.1, .1);

#define ADD_ERASER_BUTTON(sprite, pos, size) m_buttons.push_back(BrushButtonPtr(new \
    BrushButton(Sprite::GetByName(sprite), pos, size, \
                BrushPtr(new EraserBrush(Sprite::GetByName(sprite))))))

#define ADD_AREA_FIELD_BUTTON(sprite, pos, size) m_buttons.push_back(BrushButtonPtr(new \
    BrushButton(Sprite::GetByName(sprite), pos, size, \
                BrushPtr(new AreaFieldBrush(Sprite::GetByName(sprite))))))

#define ADD_SETPLAYER_BUTTON(sprite, pos, size) m_buttons.push_back(BrushButtonPtr(new \
    BrushButton(Sprite::GetByName(sprite), pos, size, \
                BrushPtr(new SetPlayerBrush(Sprite::GetByName(sprite))))))

#define ADD_FIELD_BUTTON(sprite, pos, size, field) m_buttons.push_back(BrushButtonPtr(new \
    BrushButton(Sprite::GetByName(sprite), pos, size, \
                BrushPtr(new SetFieldBrush(Sprite::GetByName(sprite), field)))))

#define ADD_ENTITY_BUTTON(sprite, pos, size, field) m_buttons.push_back(BrushButtonPtr(new \
    BrushButton(Sprite::GetByName(sprite), pos, size, \
                BrushPtr(new AddEntityBrush(Sprite::GetByName(sprite), field)))))

    ADD_ERASER_BUTTON("gui_eraser", Position(0, .0), default_size*1.75);

    ADD_AREA_FIELD_BUTTON("PlatformMid", Position(.8, .1), default_size);

    ADD_SETPLAYER_BUTTON("player_stop", Position(.1, .8), default_size);

    ADD_FIELD_BUTTON("EndOfLevel",           Position(.2, .8),  default_size, FT::EndOfLevel);
    ADD_FIELD_BUTTON("PlatformTopLeft",      Position(.3, .5),  default_size, FT::PlatformTopLeft);
    ADD_FIELD_BUTTON("PlatformTop",          Position(.4, .5),  default_size, FT::PlatformTop);
    ADD_FIELD_BUTTON("PlatformTopRight",     Position(.5, .5),  default_size, FT::PlatformTopRight);
    ADD_FIELD_BUTTON("PlatformLeft",         Position(.3, .4),  default_size, FT::PlatformLeft);
    ADD_FIELD_BUTTON("PlatformMid",          Position(.4, .4),  default_size, FT::PlatformMid);
    ADD_FIELD_BUTTON("PlatformRight",        Position(.5, .4),  default_size, FT::PlatformRight);
    ADD_FIELD_BUTTON("PlatformLeftRight",    Position(.4, .15), default_size, FT::PlatformLeftRight);
    ADD_FIELD_BUTTON("PlatformLeftTopRight", Position(.4, .25), default_size, FT::PlatformLeftTopRight);

    ADD_ENTITY_BUTTON("twinshot_upgrade",  Position(.4, .8), default_size, ET::TwinShot);
    ADD_ENTITY_BUTTON("mush_stop",         Position(.8, .8), default_size, ET::Mush);

#undef ADD_FIELD_BUTTON
#undef ADD_ENTITY_BUTTON
#undef ADD_SETPLAYER_BUTTON
#undef ADD_AREA_FIELD_BUTTON
#undef ADD_ERASER_BUTTON
}

void EditorGui::Draw() {
    // ciemne tło
    Engine::Get().GetRenderer()->DrawQuad(0, 0, 1, 1, Rgba(0,0,0,.7));

    // widoczne kontrolki
    for(const auto& button : m_buttons) {
        if (button->IsVisible() && button!=m_hovered_button) {
            button->Draw();
        }
    }

    // "najechany" przycisk rysujemy osobno.
    if (m_hovered_button) {
        m_hovered_button->Draw();
        const Aabb& box = m_hovered_button->GetAabb();
        Engine::Get().GetRenderer()->DrawQuad(box.GetMinX(), box.GetMinY(),
                                              box.GetMaxX(), box.GetMaxY(),
                                              Rgba(1,1,1, .4));
    }
}

void EditorGui::Update(double dt) {
    boost::for_each(m_buttons, boost::bind(&gui::Widget::Update, _1, dt));
}

bool EditorGui::OnMouseMove(double x, double y) {
    const Aabb cursor_aabb = Aabb(x, y, x+.02, y+0.02);
    m_hovered_button.reset();
    for(const auto& button : m_buttons) {
        if (button->IsVisible() && button->GetAabb().Collides(cursor_aabb)) {
            m_hovered_button = button;
        }
    }
    return bool(m_hovered_button);
}

bool EditorGui::OnMouseDown(Uint8 /* button */, double /* x */, double /* y */) {
    if (!m_hovered_button) {
        return false;
    }
    m_active_brush = m_hovered_button->GetBrush();
    return true;
}
