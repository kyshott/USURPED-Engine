#include "components.hpp"
#include <map>

//All of our component implementations

CName::CName() = default;
CName::CName(std::string name) : name(name) {}

CTransform::CTransform(){ 
    position=Vec2(0.0f,0.0f);
    velocity=Vec2(0.0f,0.0f);
    scale=Vec2(1.0f,1.0f);
    angle=0;
}
CTransform::CTransform(const Vec2& position, const Vec2& velocity,float angle):position(position), velocity(velocity), angle(angle){
    scale=Vec2{1.0f,1.0f};
}


CLifespan::CLifespan(int total){
    this->total=total;
    this->remaining=total;
}

CInput::CInput(){};

CHealth::CHealth(){}
CHealth::CHealth(int m, int c): max(m), current(c){}

CEffects::CEffects() {};

CStats::CStats() {};


CWeaknesses::CWeaknesses() {};
CResistances::CResistances() {};
CLoot::CLoot() {};
CWeapons::CWeapons() {};
CMagic::CMagic() {};
CRings::CRings() {};


CItems::CItems() {};
void CItems::removeItem(ItemSpec item) {
    auto it = std::find_if(items.begin(), items.end(), [&](const ItemSpec& i) {
        return i.id == item.id;
        });

    if (it != items.end()) {
        items.erase(it);
    }
}
CBoundingBox::CBoundingBox()=default;
CBoundingBox::CBoundingBox(const Vec2& s) : size(s){}

CState::CState() = default;
CState::CState(std::string state) : state(state) {}

CInvincibility::CInvincibility(){}
CInvincibility::CInvincibility(int frames) : iframes(frames){}

CFollowPlayer::CFollowPlayer(){}
CFollowPlayer::CFollowPlayer(Vec2 h, float s) : home(h), speed(s){}

CPatrol::CPatrol(){}
CPatrol::CPatrol(std::vector<Vec2>& pos, float s) : positions(pos), speed(s){}