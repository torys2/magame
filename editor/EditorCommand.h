#ifndef MAGAME_EDITORCOMMAND_H_INCLUDED
#define	MAGAME_EDITORCOMMAND_H_INCLUDED

#include "StdAfx.h"
#include "game/entity/Entity.h"

class Editor;

class EditorCommand {
public:
    virtual ~EditorCommand() {}

public:
    virtual void Execute(Editor* editor) = 0;
    virtual void Undo(Editor* editor) = 0;

    // False, jeśli polecenie nie może zostać wykonane, bo np. przekazane
    // parametry nie pozwalają na wykonanie sensownej akcji (wtedy Undo też by
    // nie miało sensu).
    virtual bool IsReady() const = 0;
    bool IsNotReady() const { return !IsReady(); }
};
typedef boost::shared_ptr<EditorCommand> EditorCommandPtr;


class AreaFieldCommand : public EditorCommand {
public:
    explicit AreaFieldCommand(const Position& start, const Position& end)
      : m_is_ready(false),
        m_beg(start), m_end(end),
        m_saved_fields() {
    }

    virtual void Execute(Editor* editor);
    virtual void Undo(Editor* editor);
    virtual bool IsReady() const;

private:
    bool m_is_ready;         // Czy polecenie jest gotowe do wykonania
    Position m_beg, m_end;   // Początek i koniec zaznaczenia. Wsp.świata
    std::vector<FT::FieldType> m_saved_fields;  // zapisane pola planszy
};


class SetPlayerCommand : public EditorCommand {
public:
    explicit SetPlayerCommand(Position pos);
    virtual void Execute(Editor* editor);
    virtual void Undo(Editor* editor);
    virtual bool IsReady() const;

private:
    Position m_new_pos;
    LevelEntityData m_saved_data;
};


class SetFieldCommand : public EditorCommand {
public:
    explicit SetFieldCommand(Position pos, FT::FieldType field)
      : m_pos(pos),
        m_field(field),
        m_saved_field(FT::None) {
    }

    virtual void Execute(Editor* editor);
    virtual void Undo(Editor* editor);
    virtual bool IsReady() const;

private:
    Position m_pos;
    FT::FieldType m_field, m_saved_field;
};


class AddEntityCommand : public EditorCommand {
public:
    explicit AddEntityCommand(Position pos, ET::EntityType type)
      : m_pos(pos),
        m_entity_type(type),
        m_entity() {
    }

    virtual void Execute(Editor* editor);
    virtual void Undo(Editor* editor);
    virtual bool IsReady() const;

private:
    Position m_pos;
    LevelEntityData m_entity_data;
    ET::EntityType m_entity_type;
    EntityPtr m_entity;
};

#endif
