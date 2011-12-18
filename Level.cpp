#include "StdAfx.h"

#include "Engine.h"
#include "Level.h"


Level::Level()
    : m_name("unknown"),
      m_width(0),
      m_height(0),
      m_loaded(false) {
}

Level::Level(LevelPtr level, const std::list<LevelEntityData>& entities_data, const LevelEntityData& player_data)
    : m_name(level->GetName()),
      m_width(level->GetWidth()),
      m_height(level->GetHeight()),
      m_data(level->m_data),
      m_entities_to_create(entities_data),
      m_player_data(player_data),
      m_loaded(true) {
    m_entities_to_create.sort(LevelEntityData());
    if (!(m_player_data.name == "player" && m_player_data.x > 0 && m_player_data.y > 0)) {
        m_player_data.name = "player";
        m_player_data.x = 1;
        m_player_data.y = 19;
    }
}


void Level::SaveFieldsToFile(const std::string& filename) {
    std::ofstream outfile(filename.c_str());
    if (!outfile) {
        std::cerr << "Nie udało się zapisać podłoża do pliku `"
                  << filename << "`. Problem z dostępem do pliku" << std::endl;
        return ;
    }

    outfile << GetWidth() << " " << GetHeight() << std::endl;
    for (size_t y = 0; y < GetHeight(); ++y) {
        for (size_t x = 0; x < GetWidth(); ++x) {
            outfile << std::setw(5) << std::setfill(' ') << m_data.at(y).at(x) << " ";
        }
        outfile << std::endl;
    }
}

void Level::SaveEntitiesToFile(const std::string& filename) {
    std::ofstream outfile(filename.c_str());
    if (!outfile) {
        std::cerr << "Nie udało się zapisać jednostek do pliku `"
                  << filename << "`. Problem z dostępem do pliku" << std::endl;
        return ;
    }
    outfile.precision(3);
    outfile << m_player_data.name << "\t" << m_player_data.x << "\t" << m_player_data.y << std::endl;
    for (std::list<LevelEntityData>::const_iterator it=m_entities_to_create.begin();
         it != m_entities_to_create.end(); ++it) {
        outfile << (*it).name << "\t" << (*it).x << "\t" << (*it).y << std::endl;
    }
    outfile.close();
}


void Level::LoadFromFile(const std::string& filename) {
    m_name = filename;
        
    std::ifstream lvl(filename.c_str());
    if (!lvl) {
        std::cerr << "Nie udało się załadować pliku " << filename << "\n";
        m_loaded = false;
        return;
    }

    lvl >> m_width >> m_height;
    m_data.resize(m_height);
    for (size_t y = 0; y < m_height; ++y) {
        m_data.at(y).resize(m_width);
    }

    for (size_t y = 0; y < m_height; ++y) {
        for (size_t x = 0; x < m_width; ++x) {
            int tmp;
            lvl >> tmp;
            m_data.at(y).at(x) = FT::FieldType(tmp);
        }
    }

    m_loaded = true;
}


void Level::LoadEntitiesFromFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file) {
        std::cerr << "Nie udało się załadować pliku " << filename << "\n";
        return;
    }

    // wczytaj linia po linii
    const int buffer_size = 1024;
    char buffer[buffer_size];
    while (file) {
        file.getline(buffer, buffer_size);
        std::string line(buffer);
        if (line.length() < 5 || line.at(0) == '#')
            continue;
        std::istringstream iss(line);
        LevelEntityData data;
        iss >> data.name;
        iss >> data.x;
        iss >> data.y;
        if (data.name == "player") {
            m_player_data = data;
        } else {
            m_entities_to_create.push_back(data);
        }

        // std::cout << "[LoadEntityFromFile] " << data.name << ", " << data.x << ", " << data.y << std::endl;
    }

    // posortuj wczytane rekordy
    m_entities_to_create.sort(LevelEntityData());
}


FT::FieldType Level::Field(size_t x, size_t y) const {
    if (x >= m_width || y >= m_height) {
        return FT::None;
    }
    return m_data.at(y).at(x);
}


bool Level::IsFieldCollidable(int x, int y) const {
    const FT::FieldType type = Field(x,y);
    return (FT::COLLIDING_START < type && type < FT::COLLIDING_END)
        || type==FT::EndOfLevel;
}

bool Level::IsFieldNotCollidable(int x, int y) const {
    return !IsFieldCollidable(x, y);
}


void Level::SetField(size_t x, size_t y, FT::FieldType ft) {
    if (x >= m_width || y >= m_height) {
        return ;
    }
    m_data.at(y).at(x) = ft;
}


Aabb Level::GetFieldAabb(size_t x, size_t y) const {
    RendererPtr renderer = Engine::Get().GetRenderer();
    const size_t v_tiles_count = renderer->GetVerticalTilesOnScreenCount();
    y = v_tiles_count - y;  // odbij y w pionie (y=0 będzie na dole)

    if (y == 0) {
        // odjęcie jedynki, gdy y==0 spowoduje przekręcenie się licznika. W tym przypadku
        // możemy zwrócić śmieć, gdyż wtedy postać i tak jest pod poniżej pierwszego kafelka,
        // a zatem za chwilę umrze.
        return Aabb(x, y, x + 1, y + 1);
    }
    Aabb box = Aabb(x, y-1, x + 1, y);
    return box;
}


void Level::EnsureWidth(size_t width) {
    if (GetWidth() >= width) {
        return;
    }
    assert(m_height > 0);  // póki co, program może zachowywać się dziwnie 
                           // dla niedodatniej wysokości poziomu.
    // std::cout << "resising from " << m_data.at(0).size() << " to " << width << std::endl;

    for (size_t y = 0; y < m_height; ++y) {
        m_data.at(y).resize(width, FT::None);  // dodaj puste komórki
    }
    m_width = width;
}

void Level::ShrinkWidth(size_t width) {
    // width == 0 oznacza automatyczne przycięcie
    if (width >= GetWidth()) {
        return ;
    }
    
    if (width > 0) {
        for (size_t y = 0; y < m_height; ++y) {
            m_data.at(y).resize(width);
        }
    } else { // automatyczne przycięcie -- znalezienie najdalszego pola poziomu
        size_t max_x = 0;
        for (size_t y = 0; y < m_height; ++y) {
            for (size_t x = 0; x < m_width; ++x) {
                if (m_data.at(y).at(x) != FT::None && x > max_x) {
                    max_x = x;
                }
            }
        }
        // najdalsza jednostka
        // TODO: implement

        max_x++;
        m_width = max_x;
        // std::cout << "shrinking to " << max_x << std::endl;
        for (size_t y = 0; y < m_height; ++y) {
            m_data.at(y).resize(max_x);
        }
    }
}