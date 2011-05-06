#include "../StdAfx.h"
#include "../Engine.h"
#include "../Text.h"
#include "../Utils.h"
#include "../Entity.h"
#include <boost/pointer_cast.hpp>
#include "EditorCommand.h"
#include "Editor.h"


void Editor::Start() {
    m_gui->Start();
}

void Editor::Init() {
    m_level_view.StoreSprite(FT::PlatformTopLeft,        Sprite::GetByName("PlatformTopLeft"));
    m_level_view.StoreSprite(FT::PlatformLeft,           Sprite::GetByName("PlatformLeft"));
    m_level_view.StoreSprite(FT::PlatformMid,            Sprite::GetByName("PlatformMid"));
    m_level_view.StoreSprite(FT::PlatformTop,            Sprite::GetByName("PlatformTop"));
    m_level_view.StoreSprite(FT::PlatformLeftTopRight,   Sprite::GetByName("PlatformLeftTopRight"));
    m_level_view.StoreSprite(FT::PlatformLeftRight,      Sprite::GetByName("PlatformLeftRight"));
    m_level_view.StoreSprite(FT::PlatformTopRight,       Sprite::GetByName("PlatformTopRight"));
    m_level_view.StoreSprite(FT::PlatformRight,          Sprite::GetByName("PlatformRight"));

    m_level_view.StoreSprite(FT::EndOfLevel,             Sprite::GetByName("EndOfLevel"));

    m_level_view.StoreSprite(FT::NcPlatformTopLeft,      Sprite::GetByName("NcPlatformTopLeft"));
    m_level_view.StoreSprite(FT::NcPlatformLeft,         Sprite::GetByName("NcPlatformLeft"));
    m_level_view.StoreSprite(FT::NcPlatformMid,          Sprite::GetByName("NcPlatformMid"));
    m_level_view.StoreSprite(FT::NcPlatformTop,          Sprite::GetByName("NcPlatformTop"));
    m_level_view.StoreSprite(FT::NcPlatformLeftTopRight, Sprite::GetByName("NcPlatformLeftTopRight"));
    m_level_view.StoreSprite(FT::NcPlatformLeftRight,    Sprite::GetByName("NcPlatformLeftRight"));
    m_level_view.StoreSprite(FT::NcPlatformTopRight,     Sprite::GetByName("NcPlatformTopRight"));
    m_level_view.StoreSprite(FT::NcPlatformRight,        Sprite::GetByName("NcPlatformRight"));

    m_gui->Init();
}

void Editor::Draw() {
    if (IsInGame()) {
        m_game->Draw();
        return;
    }
    if (IsClearBeforeDraw()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
    }

    glPushAttrib(GL_COLOR_BUFFER_BIT);
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const double tile_width = Engine::Get().GetRenderer()->GetTileWidth();
        const double viewer_x   = -(m_viewer_offset_x * tile_width - 0.45);

        DrawEntitiesPlayerAndLevel(viewer_x);
        DrawBrushAndGui(viewer_x);
    }
    glPopAttrib();

    if (IsSwapAfterDraw()) {
        SDL_GL_SwapBuffers();
    }
}

void Editor::DrawEntitiesPlayerAndLevel(double viewer_x) {
    const double tile_width  = Engine::Get().GetRenderer()->GetTileWidth();
    const double tile_height = Engine::Get().GetRenderer()->GetTileHeight();

    glPushMatrix();
    {
        glTranslated(viewer_x, 0, 0);

        // jednostki i gracz
        std::for_each(m_entities.begin(), m_entities.end(), boost::bind(&Entity::Draw, _1));
        if (m_player_data.x >= 0) {
            const SpritePtr player_sprite = Sprite::GetByName("player_stop");
            const double player_x(m_player_data.x * tile_width), player_y(m_player_data.y * tile_height);
            player_sprite->DrawCurrentFrame(player_x, player_y, tile_width, tile_height);
        } else {
            // nie ma danych gracza, to znaczy, że nie został ustawiony. Dlatego
            // nie rysujemy jego sprite'a. Przy przejściu do podglądu gry zostanie
            // dodany na domyślnej pozycji.
        }

        // poziom
        double offset = m_viewer_offset_x;
        m_level_view.SetLevel(m_level, offset);
        m_level_view.Draw(offset);
    }
    glPopMatrix();
}

void Editor::DrawBrushAndGui(double viewer_x) {
    const double tile_width  = Engine::Get().GetRenderer()->GetTileWidth();
    const double tile_height = Engine::Get().GetRenderer()->GetTileHeight();

    glPushAttrib(GL_ENABLE_BIT);
    {
        // wyłącz test głębokości, żeby pędzel oraz gui były zawsze na wierzchu
        glDisable(GL_DEPTH_TEST);
        SetBrush(m_gui->GetActiveBrush());
        if (GetBrush()) {
            glPushMatrix();
            {
                glTranslated(viewer_x, 0, 0);
                Size size(tile_width, tile_height);
                Position position(m_pointer_x * tile_width, m_pointer_y * tile_height);
                // niektóre pola są wyrównywane do siatki, więc trzeba przyciąć współrzędne
                if (ShouldSnapToGrid()) {
                    position = Position(static_cast<int>(m_pointer_x) * tile_width,
                                        static_cast<int>(m_pointer_y) * tile_height);
                }
                Engine::Get().GetRenderer()->DrawQuad(position, position+size, 1,1,1,.4); // podświetlenie
                GetBrush()->Draw(position, size);
            }
            glPopMatrix();
        }
        if (IsGuiVisible()) {
            m_gui->Draw();
        }
    }
    glPopAttrib();
}

bool Editor::ShouldSnapToGrid() const {
    if (InPaintingFieldMode())
        return true;
    if (InPaintingSpecialMode()
        && GetBrush()->IsSpecial()
        && GetBrush()->GetSpecialType()==Brush::ST::Eraser)
        return true;
    return false;
}

bool Editor::Update(double dt) {
    if (IsInGame()) {
        m_game->Update(dt);
        if (m_game->IsDone()) {
            SwitchToEditor();
        }
        return !IsDone();
    }

    if (m_keys_down[SDLK_ESCAPE]) {
        SetDone(true);
    } else if (m_keys_down[SDLK_LEFT]) {
        m_viewer_offset_x -= dt * 28.19;
    } else if (m_keys_down[SDLK_RIGHT]) {
        m_viewer_offset_x += dt * 28.24;
    }

    // upewnij się że edytor nie zagląda poza lewą krawędź planszy. Znajdują się tam
    // ujemne wartości na osi odciętych, co może powodować błędy w obliczeniach.
    const double tiles_in_row = 1.0/Engine::Get().GetRenderer()->GetTileWidth();
    m_viewer_offset_x = std::max(m_viewer_offset_x, tiles_in_row/2-1);

    // aktualizacja GUI
    m_gui->Update(dt);

    return !IsDone();
}

void Editor::ReleaseAtCoords(double x, double y) {
    BrushPtr brush = m_gui->GetActiveBrush();
    if (brush && (brush->GetSpecialType() == Brush::ST::Multi)) {
        MultiBrushPtr multibrush = boost::dynamic_pointer_cast<MultiBrush>(brush);
        std::cerr << "move at: " << x << ", " << y << std::endl;
        multibrush->FinishAt(x, y);
    }
}

void Editor::MoveToCoords(double x, double y) {
    BrushPtr brush = m_gui->GetActiveBrush();
    if (brush && (brush->GetSpecialType() == Brush::ST::Multi)) {
        MultiBrushPtr multibrush = boost::dynamic_pointer_cast<MultiBrush>(brush);
        multibrush->MoveTo(x, y);
    }
}

void Editor::ActionAtCoords(double x, double y) {
    BrushPtr brush = m_gui->GetActiveBrush();
    if (brush) {
        if (InPaintingFieldMode()) {
            SetFieldAt(static_cast<size_t>(x), static_cast<size_t>(y), brush->GetFieldType());
        } else if (InPaintingEntityMode()) {
            const ET::EntityType entity_type = brush->GetEntityType();
            assert(entity_type!=ET::UNKNOWN);
            assert(entity_type!=ET::COUNT);
            const std::string name = EntityTypeAsString(entity_type);
            const LevelEntityData entity_data(name, x, y);
            m_entities_to_create.push_back(entity_data);
            EntityFactory factory;
            m_entities.push_back(factory.CreateEntity(entity_data));
            // std::cout << "New entity: " << name << " " << x << " " << y << std::endl;
        } else if (InPaintingSpecialMode()) {
            const Brush::ST::SpecialType special_type = brush->GetSpecialType();
            if (special_type == Brush::ST::Player) {
                m_player_data = LevelEntityData("player", x, y);
            } else if (special_type == Brush::ST::Eraser) {
                ClearFieldAt(static_cast<size_t>(x), static_cast<size_t>(y));
            } else if (special_type == Brush::ST::Multi) {
                std::cerr << "Akcja typu MULTI" << std::endl;
                MultiBrushPtr multibrush = boost::dynamic_pointer_cast<MultiBrush>(brush);
                multibrush->StartAt(x, y);
                EditorCommandPtr command = brush->GetCommand();
                command->Execute();
                m_commands.push_back(command);

            } else {
                std::cerr << "Niezdefiniowana akcja w trybie specjalnym" << std::endl;
            }
        }
        else {
            std::cerr << "Nie odnaleziono trybu rysowania" << std::endl;
            assert(false && "Nie odnaleziono trybu rysowania");
        }
    }
}

void Editor::ProcessEvents(const SDL_Event& event) {
    if (IsDone()) {
        return;
    }

    if (event.type == SDL_KEYUP && event.key.keysym.sym==SDLK_0) {
        if (IsInGame()) {
            SwitchToEditor();
        } else {
            LevelPtr level(new Level(m_level, m_entities_to_create, m_player_data));
            level->ShrinkWidth();
            level->SaveEntitiesToFile("data/new.ents");
            level->SaveFieldsToFile("data/new.lvl");
            m_game.reset(new Game(level, PlayerPtr()));
            m_game->Init();
            m_game->Start();
            SwitchToGame();
        }
        return;
    }
    if (IsInGame()) {
        m_game->ProcessEvents(event);
        return;
    }

    if (event.type == SDL_QUIT) {
        SetDone();
    } else if (event.type == SDL_KEYDOWN) {
        if (IsGuiHidden() || m_gui->OnKeyDown(event.key.keysym.sym)==false) {
            m_keys_down[event.key.keysym.sym] = true;
        }
    } else if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym==SDLK_1) {
            ToggleGui();
        } else if (IsGuiHidden() || m_gui->OnKeyUp(event.key.keysym.sym)==false) {
            m_keys_down[event.key.keysym.sym] = false;
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        m_pointer_window_x =       event.motion.x / static_cast<double>(Engine::Get().GetWindow()->GetWidth());
        m_pointer_window_y = 1.0 - event.motion.y / static_cast<double>(Engine::Get().GetWindow()->GetHeight());
        if (IsGuiVisible() && m_gui->OnMouseMove(m_pointer_window_x, m_pointer_window_y)) {
            m_pointer_x = m_pointer_y = 1000;   // przesuń kursor poza ekran (można to zrobić ładniej)
        } else {
            m_pointer_x = MapWindowCoordToWorldX(m_pointer_window_x);
            m_pointer_y = MapWindowCoordToWorldY(m_pointer_window_y);
            MoveToCoords(m_pointer_x, m_pointer_y);
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        m_pointer_window_x =       event.motion.x / static_cast<double>(Engine::Get().GetWindow()->GetWidth());
        m_pointer_window_y = 1.0 - event.motion.y / static_cast<double>(Engine::Get().GetWindow()->GetHeight());
        if (IsGuiVisible() && m_gui->OnMouseDown(event.button.button, m_pointer_window_x, m_pointer_window_y)) {
        } else {
            m_pointer_x = MapWindowCoordToWorldX(m_pointer_window_x);
            m_pointer_y = MapWindowCoordToWorldY(m_pointer_window_y);
            ActionAtCoords(m_pointer_x, m_pointer_y);
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        m_pointer_x = MapWindowCoordToWorldX(m_pointer_window_x);
        m_pointer_y = MapWindowCoordToWorldY(m_pointer_window_y);
        ReleaseAtCoords(m_pointer_x, m_pointer_y);
    }
}

double Editor::MapWindowCoordToWorldX(double x) const {
    const double tiles_in_row = 1.0/Engine::Get().GetRenderer()->GetTileWidth();
    double k = x*tiles_in_row + m_viewer_offset_x - tiles_in_row/2 + 1;
    return k;
}

double Editor::MapWindowCoordToWorldY(double y) const {
    const double th = Engine::Get().GetRenderer()->GetTileHeight();
    return y/th;
}

void Editor::ClearFieldAt(double x, double y) {
    SetFieldAt(x, y, FT::None);
}

void Editor::SetFieldAt(double x, double y, FT::FieldType ft) {
    m_level->EnsureWidth(static_cast<size_t>(x+1));
    m_level->SetField(static_cast<size_t>(x), static_cast<size_t>(TopDown(y)), ft);
}

FT::FieldType Editor::GetFieldAt(double x, double y) const {
    return m_level->Field(static_cast<size_t>(x), static_cast<size_t>(TopDown(y)));
}
