#ifndef __APP_STATE_H__
#define __APP_STATE_H__

#include <SDL/SDL.h>
#include <boost/shared_ptr.hpp>

class AppState {
public:
    explicit AppState() 
        : m_is_done(false) {
    }

    virtual void Init() = 0;
    virtual void Start() = 0;

    virtual void Draw() = 0;
    virtual bool Update(double dt) = 0;
    virtual void ProcessEvents(const SDL_Event& event) = 0;

    virtual boost::shared_ptr<AppState> NextAppState() const = 0;

    bool IsDone() const {
        return m_is_done;
    }

    void SetDone() {
        m_is_done = true;
    }

private:
    bool m_is_done;
};

typedef boost::shared_ptr<AppState> AppStatePtr;

#endif /* __APP_STATE_H__ */
