#include "StdAfx.h"

#include "Entity.h"
#include "Engine.h"

Entity::Entity(double x, double y, double def_velocity_x, double def_velocity_y,
               double def_acceleration_x, double def_acceleration_y)
    : m_default_velocity_x(def_velocity_x),
      m_default_velocity_y(def_velocity_y),
      m_default_acceleration_x(def_acceleration_x),
      m_default_acceleration_y(def_acceleration_y),
      m_is_dead(false),
      m_state(ES::Stand),
      m_x(x),
      m_y(y),
      m_vx(0.0),
      m_vy(0),
      m_ax(def_acceleration_x),
      m_ay(def_acceleration_y),
      m_is_on_ground(true),
      m_can_go_left(true),
      m_can_go_right(true)
      {
    SetDefaultMovement();
}

void Entity::SetSprites(SpritePtr left, SpritePtr right, SpritePtr stop) {
    m_left = left;
    m_right = right;
    m_stop = stop;
}

bool Entity::IsAnyFieldBelowMe(double dt, LevelPtr level) const {
    size_t curr_x_tile, curr_y_tile;
    GetCurrentTile(&curr_x_tile, &curr_y_tile);
    for (int x = -1; x < 2; ++x) {
        if (level->IsFieldNotCollidable(curr_x_tile + x, curr_y_tile + 1)) {
            continue;
        }
        const Aabb field_aabb = level->GetFieldAabb(curr_x_tile + x, curr_y_tile + 1);
        if (GetNextVerticalAabb(dt).IsOver(field_aabb)) {
            return true;
        }
    }
    return false;
}

bool Entity::IsAnyFieldAboveMe(double dt, LevelPtr level) const {
    size_t curr_x_tile, curr_y_tile;
    GetCurrentTile(&curr_x_tile, &curr_y_tile);
    for (int x = -1; x < 2; ++x) {
        if (level->IsFieldNotCollidable(curr_x_tile + x, curr_y_tile - 1)) {
            continue;
        }
        const Aabb field_aabb = level->GetFieldAabb(curr_x_tile + x, curr_y_tile - 1);
        if (GetNextVerticalAabb(dt).IsUnder(field_aabb)) {
            return true;
        }
    }
    return false;
}

bool Entity::IsAnyFieldOnLeft(double dt, LevelPtr level) const {
    size_t curr_x_tile, curr_y_tile;
    GetCurrentTile(&curr_x_tile, &curr_y_tile);
    for (int y = -1; y < 2; ++y) {
        if (level->IsFieldNotCollidable(curr_x_tile - 1, curr_y_tile + y)) {
            continue;
        }
        const Aabb field_aabb = level->GetFieldAabb(curr_x_tile - 1, curr_y_tile + y);
        if (GetNextHorizontalAabb(dt).IsOnRightOf(field_aabb)) {
            return true;
        }
    }
    return false;
}

bool Entity::IsAnyFieldOnRight(double dt, LevelPtr level) const {
    size_t curr_x_tile, curr_y_tile;
    GetCurrentTile(&curr_x_tile, &curr_y_tile);
    for (int y = -1; y < 2; ++y) {
        if (level->IsFieldNotCollidable(curr_x_tile + 1, curr_y_tile + y)) {
            continue;
        }
        const Aabb field_aabb = level->GetFieldAabb(curr_x_tile + 1, curr_y_tile + y);
        if (GetNextHorizontalAabb(dt).IsOnLeftOf(field_aabb)) {
            return true;
        }
    }
    return false;
}

bool Entity::DoFieldsEndOnLeft(double dt, LevelPtr level) const {
    size_t curr_x_tile, curr_y_tile;
    GetCurrentTile(&curr_x_tile, &curr_y_tile);
    // czy pole o jeden w lewo i w doł jest puste (None)
    // oraz czy jednostka jest przy krawędzi (IsOnLeftOf)...
    if (level->IsFieldNotCollidable(curr_x_tile - 1, curr_y_tile + 1)
        && level->GetFieldAabb(curr_x_tile - 1, curr_y_tile).IsOnLeftOf(GetNextHorizontalAabb(dt))) {
        return true;
    }
    return false;
}

bool Entity::DoFieldsEndOnRight(double dt, LevelPtr level) const {
    size_t curr_x_tile, curr_y_tile;
    GetCurrentTile(&curr_x_tile, &curr_y_tile);
    if (level->IsFieldNotCollidable(curr_x_tile + 1, curr_y_tile + 1)
        && level->GetFieldAabb(curr_x_tile + 1, curr_y_tile).IsOnRightOf(GetNextHorizontalAabb(dt))) {
        return true;
    }
    return false;
}

void Entity::CheckCollisionsWithLevel(double dt, LevelPtr level) {
    // czy jednostka koliduje z czymś od góry
    if(IsAnyFieldAboveMe(dt, level)) {
        Fall();  // rozpocznij spadanie
    }

    // czy jednostka koliduje z czymś od dołu
    if(IsAnyFieldBelowMe(dt, level)) {
        EntityOnGround();   // zatrzymaj na podłożu
    }

    // czy jednostka koliduje z czymś po lewej stronie
    if(IsAnyFieldOnLeft(dt, level)) {
        NegateXVelocity();  // zawróć
    }

    // czy jednostka koliduje z czymś po prawej stronie
    if(IsAnyFieldOnRight(dt, level)) {
        NegateXVelocity();  // zawróć
    }
}

ES::EntityState Entity::SetStateFromVelocity(double velocity_x) {   
    if (fabs(velocity_x) < 0.0001) {
        m_state = ES::Stand;
    } else if (velocity_x > 0.0) {
        m_state = ES::GoRight;
    } else {
        m_state = ES::GoLeft;
    }
    return m_state;
}

void Entity::UpdateSpriteFromState(double dt) {
    switch (m_state) {
    case ES::Stand:
        if (m_stop) m_stop->Update(dt);
        break;
    case ES::GoLeft:
        if (m_left) m_left->Update(dt);
        break;
    case ES::GoRight:
        if (m_right) m_right->Update(dt);
        break;
    }
}


void Entity::CalculateNextXPosition(double dt) {
    double next_x = GetNextXPosition(dt);
    if (next_x < m_x && m_can_go_left) {
        m_x = next_x;
    } else if (next_x > m_x && m_can_go_right) {
        m_x = next_x;
    }
}


void Entity::Update(double dt, LevelPtr level) {
    // ustaw domyślny ruch i sprawdź czy co w świecie piszczy
    SetDefaultMovement();
    CheckCollisionsWithLevel(dt, level);

    // wylicz nową prędkość oraz połóżenie na osi OY
    if (!m_is_on_ground) {
        m_y = GetNextYPosition(dt);
        m_vy += m_ay * dt;
    }

    // jeżeli poniżej pierwszego kafla, to nie spadaj niżej.
    // Na razie ustalamy poziom na y=1, aby jednostka nie uciekała za ekran
    if (m_y < 0) {
        m_y = 0;
        EntityOnGround();
    }

    // wylicz pozycję gracza w poziomie (oś OX).
    CalculateNextXPosition(dt);

    // nie można wyjść poza mapę
    if (m_x < 1) {
        m_x = 1; // nie można wyjść za początek mapy
    }

    // ustal stan ruchu gracza na podstawie prędkości
    SetStateFromVelocity(m_vx);
    if (m_state == ES::Stand) {
        m_vx = 0;
    }

    // uaktualnij animację
    UpdateSpriteFromState(dt);
}

void Entity::Draw() const {
    // wylicz pozycję gracza na ekranie
    const Size tile_size = Engine::Get().GetRenderer()->GetTileSize();
    const Position entity_pos(m_x * tile_size.X(), m_y * tile_size.Y());

    switch (m_state) {
    case ES::Stand:
        if (m_stop) m_stop->DrawCurrentFrame(entity_pos, tile_size);
        break;
    case ES::GoLeft:
        if (m_left) m_left->DrawCurrentFrame(entity_pos, tile_size);
        break;
    case ES::GoRight:
        if (m_right) m_right->DrawCurrentFrame(entity_pos, tile_size);
        break;
    }

//    Engine::Get().GetRenderer()->DrawAabb(GetAabb());
}

void Entity::KilledByPlayer() {
    SetIsDead(true);
    // std::cout << "[Entity] Killed by player" << std::endl;
}

void Entity::KilledWithBullet() {
    SetIsDead(true);
    // std::cout << "[Entity] Killed by bullet" << std::endl;
}
