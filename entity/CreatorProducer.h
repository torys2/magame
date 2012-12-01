#ifndef MAGAME_CREATOR_PRODUCER_H_INCLUDED
#define MAGAME_CREATOR_PRODUCER_H_INCLUDED
#include "StdAfx.h"

#include "Creator.h"

// Klasa bazowa dla klas, które będą dodawać kreatory
class CreatorProducer {
public:
    virtual ~CreatorProducer() {}

    void AddCreator(CreatorPtr creator) {
        m_creators.push_back(creator);
    }

    void DropAllCreators() {
        m_creators.clear();
    }

    std::list<CreatorPtr> GetCreators() const {
        return m_creators;
    }

    std::list<CreatorPtr> GetAndDropCreators() {
        std::list<CreatorPtr> cs = GetCreators();
        DropAllCreators();
        return cs;
    }

private:
    std::list<CreatorPtr> m_creators;
};

#endif
