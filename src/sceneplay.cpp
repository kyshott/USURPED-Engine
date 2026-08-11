#include "sceneplay.hpp"
#include "scenemenu.hpp"
#include <fstream>
#include <iostream>
#include <cstdio>
#include "physics.hpp"
#include <string>
#include <print>
#include "scenebattle.hpp"
#include "tileson.hpp"

/**
 * Constructor for the Play Scene
 * 
 * @param gameEngine raw pointer to the game engine class
 * @param levelPath Path to level defintion file, relative to exe
 * @param first Indicator of whether this is the first floor or not
 * @param stage Indicator of stage level
 */
ScenePlay::ScenePlay(GameEngine* gameEngine, std::string& levelPath, bool first, int stage):Scene(gameEngine){
    this->levelPath = levelPath;
    this->first = first;
	this->stage = stage;
    init(this->levelPath);
}

/*
* Alternative constructor for play scene for subsequent floors.
* 
* @param gameEngine raw pointer to the game engine class
* @param levelPath Path to level definition file, relative to exe
* @param player Shared pointer to the player entity, passed along to the new scene
* @param first Indicator of whether this is the first floor or not
* @param stage Indicator of stage level
*/
ScenePlay::ScenePlay(GameEngine* gameEngine, std::string& levelPath, std::shared_ptr<Entity> player, bool first, int stage):Scene(gameEngine) {
    this->levelPath = levelPath;
    this->first = first;
    this->player = player;
    this->stage = stage;    
    init(this->levelPath);
}

/**
 * Functions that initializes the scene:
 * 
 * 1. Loads Level information
 * 2. Spawns Players  
 * 3. Registers input action  
 * 4. Sets up camera  
 * 
 * @param levelPath Path to level defintion file, relative to exe
 */
void ScenePlay::init(const std::string& levelPath){
    loadMap(levelPath);
    renderTiledMap(levelPath);
    if (first) {
        spawnPlayer();
    }
    else {
        entityManager.addExistingEntity(player);
        player->addComponent<CTransform>(spawnPoint, Vec2(0.0f, 0.0f), 0.0f);
        player->getComponent<CStats>().stage += 1;
    }
	buildInventoryMenu();

    menuStrings.push_back("STATUS");
    menuStrings.push_back("ITEMS");
	menuStrings.push_back("MAGIC");
    menuStrings.push_back("WEAPONS");
    menuStrings.push_back("QUIT");

    registerAction(KEY_C, "CAMERA");

    registerAction(KEY_W, "UP");
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_A, "LEFT");
    registerAction(KEY_D, "RIGHT");
    registerAction(KEY_SPACE, "INTERACT");
	registerAction(KEY_TAB, "INVENTORY");
	registerAction(KEY_BACKSPACE, "BACK");
    
    mainCamera=Camera2D({gameEngine->getWidth()/2.0f*GetWindowScaleDPI().x,gameEngine->getHeight()/2.0f*GetWindowScaleDPI().y},{gameEngine->getWidth()/2.0f,gameEngine->getHeight()/2.0f},0,GetWindowScaleDPI().x);
    gameEngine->playMusic("TITLEMUSIC");
}

/*
* Loads, builds and renders the map from the Tiled JSON file.
* 
* @param levelPath Path to the Tiled JSON file
*/
void ScenePlay::loadMap(const std::string& levelPath) {
    tson::Tileson t;
	std::shared_ptr<tson::Map> map = t.parse(levelPath);

   if (map->getStatus() == tson::ParseStatus::OK) {
       
       for (auto& layer : map->getLayers()) {
           if (layer.getType() == tson::LayerType::ObjectGroup) {
               for (auto& obj : layer.getObjects()) {
                   if (obj.getType() == "ENEMY") {
                       auto e = entityManager.addEntity("DYNAMIC", "ENEMY");
                       e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(obj.getName()), true);
					   gameEngine->getAssets().getEnemy(obj.getName(), e, this->stage);
					   e->addComponent<CTransform>(Vec2(obj.getPosition().x * 4, obj.getPosition().y * 4), Vec2(0.0f, 0.0f), 0.0f);
                       e->getComponent<CTransform>().prevPosition.x = obj.getPosition().x * 4;
                       e->getComponent<CTransform>().prevPosition.y = obj.getPosition().y * 4;
					   e->addComponent<CBoundingBox>(gameEngine->getAssets().getAnimation(obj.getName()).getScaledSize());
                       if (e->hasComponent<CFollowPlayer>()) {
                           e->getComponent<CFollowPlayer>().base = e->getComponent<CTransform>().position;
                       }
                   }
                   if (obj.getType() == "BOSS") {
                       auto e = entityManager.addEntity("DYNAMIC", "BOSS");
                       e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(obj.getName()), true);
                       gameEngine->getAssets().getEnemy(obj.getName(), e, this->stage);
                       e->addComponent<CTransform>(Vec2(obj.getPosition().x * 4, obj.getPosition().y * 4), Vec2(0.0f, 0.0f), 0.0f);
                       e->getComponent<CTransform>().prevPosition.x = obj.getPosition().x * 4;
                       e->getComponent<CTransform>().prevPosition.y = obj.getPosition().y * 4;
                       e->addComponent<CBoundingBox>(gameEngine->getAssets().getAnimation(obj.getName()).getScaledSize());
                       if (e->hasComponent<CFollowPlayer>()) {
                           e->getComponent<CFollowPlayer>().base = e->getComponent<CTransform>().position;
                       }
                   }
                   if (obj.getType() == "CHEST") {
					   auto e = entityManager.addEntity("INTERACTABLE", "CHEST");
                       if (obj.getName() == "CHEST") {
                           e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("CHEST"), true);
                           e->addComponent<CItems>();
                           std::string loot = selectLoot("ITEM");
						   e->getComponent<CItems>().items.push_back(gameEngine->getAssets().getItem(loot));
                           e->addComponent<CState>();
                           e->getComponent<CState>().state = "CLOSED";
                       } 
                       else if (obj.getName() == "MAGICCHEST") {
                           e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("MAGICCHEST"), true);
                           e->addComponent<CMagic>();
                           std::string loot = selectLoot("MAGIC");
                           e->getComponent<CMagic>().magic.push_back(gameEngine->getAssets().getMagic(loot));
                           e->addComponent<CState>();
                           e->getComponent<CState>().state = "CLOSED";
					   }
                       else if (obj.getName() == "WEAPONCHEST") {
                           e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("WEAPONCHEST"), true);
                           e->addComponent<CWeapons>();
                           std::string loot = selectLoot("WEAPON");
                           e->getComponent<CWeapons>().weapons.push_back(gameEngine->getAssets().getWeapon(loot));
                           e->addComponent<CState>();
                           e->getComponent<CState>().state = "CLOSED";
					   }
					   e->getComponent<CTransform>().position.x = obj.getPosition().x * 4;
                       e->getComponent<CTransform>().position.y = obj.getPosition().y * 4;
                       e->getComponent<CTransform>().prevPosition.x = obj.getPosition().x * 4;
					   e->getComponent<CTransform>().prevPosition.y = obj.getPosition().y * 4;
                       e->addComponent<CBoundingBox>(gameEngine->getAssets().getAnimation(obj.getName()).getScaledSize());
                   }
                   if (obj.getType() == "ENTRANCE") {
                       spawnPoint.x = obj.getPosition().x * 4;
					   spawnPoint.y = obj.getPosition().y * 4;
                       auto e = entityManager.addEntity("DEC", "ENTRANCE");
                       e->addComponent<CTransform>(spawnPoint, Vec2(0.0f, 0.0f), 0.0f);
                       e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("ENTRANCE"), true);
                   }
                   if (obj.getType() == "EXIT") {
                       auto e = entityManager.addEntity("DYNAMIC", "EXIT");
                       e->addComponent<CBoundingBox>(gameEngine->getAssets().getAnimation("EXIT").getScaledSize());
                       e->addComponent<CTransform>(Vec2(obj.getPosition().x * 4, obj.getPosition().y * 4), Vec2(0.0f, 0.0f), 0.0f);
                       e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("EXIT"), true);
                   }
                   if (obj.getType() == "BOSSDOOR") {
                       auto e = entityManager.addEntity("STATIC", "BOSSDOOR");
                       e->addComponent<CBoundingBox>(gameEngine->getAssets().getAnimation("BOSSDOOR").getScaledSize());
                       e->addComponent<CTransform>(Vec2(obj.getPosition().x * 4, obj.getPosition().y * 4), Vec2(0.0f, 0.0f), 0.0f);
					   e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("BOSSDOOR"), true);
                   }
               }
           }
       }
   }

   buildTiledMap(levelPath);
}

/*
* Builds the tiled map from the Tiled JSON file and creates static tile entities in the game world.
* 
* @param levelPath Path to the Tiled JSON file
*/
void ScenePlay::buildTiledMap(const std::string& levelPath) {
    tson::Tileson t;
    std::shared_ptr<tson::Map> tiledMap = t.parse(levelPath);

    for (auto& layer : tiledMap->getLayers()) {
        if (layer.getType() != tson::LayerType::TileLayer) {
            continue;
        }

        for (const auto& [tilePos, tile] : layer.getTileData()) {
            if (tile == nullptr) {
                continue;
            }

            if (!(tile->getClassType() == "TILE")) {
                continue;
            }

            const int tiledX = std::get<0>(tilePos);
            const int tiledY = std::get<1>(tilePos);

            const int engineGridY = gameEngine->getTilesY() - 1 - tiledY;

            const float centerX = tiledX * gameEngine->getTileSizeX() + gameEngine->getTileSizeX() / 2.0f;
            const float centerY = gameEngine->getHeight() - engineGridY * gameEngine->getTileSizeY() - gameEngine->getTileSizeY() / 2.0f;

            auto e = entityManager.addEntity("STATIC", "TILE");
            e->addComponent<CTransform>(Vec2(centerX, centerY), Vec2(0.0f, 0.0f), 0.0f);
			e->getComponent<CTransform>().prevPosition.x = centerX;
			e->getComponent<CTransform>().prevPosition.y = centerY;
            e->addComponent<CBoundingBox>(Vec2(gameEngine->getTileSizeX(), gameEngine->getTileSizeY()));

            bool visionProp = tile->getProp("blocksVision");
            if (visionProp == true) {
                e->getComponent<CBoundingBox>().blocksVision = visionProp;
            }

            const tson::Rect& srcRect = tile->getDrawingRect();
        }
    }
}

/*
* Draws the visible portion of the tiled map based on the camera's view.
*/
void ScenePlay::drawVisibleTiledMap() {
    if (cachedMapTiles.empty()) {
        return;
    }

    Vector2 dpi = GetWindowScaleDPI();

    Vector2 worldTopLeft = GetScreenToWorld2D(
        Vector2{ 0.0f, 0.0f },
        mainCamera
    );

    Vector2 worldBottomRight = GetScreenToWorld2D(
        Vector2{
            static_cast<float>(gameEngine->getWidth()) * dpi.x,
            static_cast<float>(gameEngine->getHeight()) * dpi.y
        },
        mainCamera
    );

    const float left = std::min(worldTopLeft.x, worldBottomRight.x);
    const float right = std::max(worldTopLeft.x, worldBottomRight.x);
    const float top = std::min(worldTopLeft.y, worldBottomRight.y);
    const float bottom = std::max(worldTopLeft.y, worldBottomRight.y);

    Rectangle visibleArea = {
        left,
        top,
        right - left,
        bottom - top
    };

    for (const auto& tile : cachedMapTiles) {
        if (!CheckCollisionRecs(tile.dest, visibleArea)) {
            continue;
        }

        const Texture2D& tilesetTexture = gameEngine->getAssets().getTexture(tile.tilesetName);

        DrawTexturePro(
            tilesetTexture,
            tile.src,
            tile.dest,
            Vector2{ 0.0f, 0.0f },
            0.0f,
            WHITE
        );
    }
}

/*
* Renders the entire tiled map and caches the tile information.
* 
* @param levelPath Path to the Tiled JSON file
*/
void ScenePlay::renderTiledMap(const std::string& levelPath) {
    cachedMapTiles.clear();

    tson::Tileson t;
    std::shared_ptr<tson::Map> tiledMap = t.parse(levelPath);

    if (tiledMap->getStatus() != tson::ParseStatus::OK) {
        return;
    }

    cachedMapTiles.reserve(tiledMap->getSize().x * tiledMap->getSize().y);

    for (auto& layer : tiledMap->getLayers()) {
        if (layer.getType() != tson::LayerType::TileLayer) {
            continue;
        }

        for (const auto& [tilePos, tile] : layer.getTileData()) {
            if (tile == nullptr) {
                continue;
            }

            tson::Tileset* tileset = tile->getTileset();
            if (tileset == nullptr) {
                continue;
            }

            const int tileX = std::get<0>(tilePos);
            const int tileY = std::get<1>(tilePos);
            const tson::Rect& srcRect = tile->getDrawingRect();

            cachedMapTiles.push_back({
                tileset->getName(),
                Rectangle{
                    static_cast<float>(srcRect.x),
                    static_cast<float>(srcRect.y),
                    static_cast<float>(srcRect.width),
                    static_cast<float>(srcRect.height)
                },
                Rectangle{
                    static_cast<float>(tileX * gameEngine->getTileSizeX()),
                    static_cast<float>(tileY * gameEngine->getTileSizeY()),
                    static_cast<float>(gameEngine->getTileSizeX()),
                    static_cast<float>(gameEngine->getTileSizeY())
                }
                });
        }
    }
}

/**
 * Animation System
 * 
 * For the player: Sets the state and animation depending on user input.
 * For other entities: Updates the animation via the CAnimation component.
 *
 */
void ScenePlay::sAnimation() {
    CState& playerState = player->getComponent<CState>();
    CAnimation& playerAnim = player->getComponent<CAnimation>();
    CTransform& transf = player->getComponent<CTransform>();

    const bool isMoving =
        std::abs(transf.velocity.x) > 0.05f ||
        std::abs(transf.velocity.y) > 0.05f;

    std::string nextAnimation = playerAnim.animation.getName();
    /* Uncomment for interact animations
    if (playerState.isInteracting) {
        if (transf.facing.y == 1) {
            nextAnimation = "OLUSEU";
        }
        else if (transf.facing.y == -1) {
            nextAnimation = "OLUSED";
        }
        else if (transf.facing.x == -1 || transf.facing.x == 1) {
            nextAnimation = "OLUSER";
        }
    }
    */

    if (transf.facing.y == 1) {
        nextAnimation = isMoving ? "OLWALKU" : "OLSTANDU";
    }
    else if (transf.facing.y == -1) {
        nextAnimation = isMoving ? "OLWALKD" : "OLSTANDD";
    }
    else if (transf.facing.x == -1 || transf.facing.x == 1) {
        nextAnimation = isMoving ? "OLWALKR" : "OLSTANDR";
    }

    if (nextAnimation != playerAnim.animation.getName()) {
        playerAnim.animation = gameEngine->getAssets().getAnimation(nextAnimation);
    }

    for (auto& e : entityManager.getEntities()) {
        if (e->hasComponent<CAnimation>()) {
            e->getComponent<CAnimation>().animation.update();
        }
    }
}

/**
 * Movement System
 * 
 * Moves all DYNAMIC entities
 * 
 */
void ScenePlay::sMovement() {
    for (auto& e : entityManager.getEntities("DYNAMIC")) {

        if (e->hasComponent<CInput>()) {
            CInput& inp = e->getComponent<CInput>();
            CTransform& transf = e->getComponent<CTransform>();
            CState& state = e->getComponent<CState>();

            const float ACCEL = 0.35f;
            const float DRAG = 0.84f;
            const float MAX_SPEED = playerConfig.SPEED;

            Vec2 inputDir(0.0f, 0.0f);
            if (inp.left)  inputDir.x -= 1.0f;
            if (inp.right) inputDir.x += 1.0f;
            if (inp.up)    inputDir.y -= 1.0f;
            if (inp.down)  inputDir.y += 1.0f;

            if (inputDir.length() > 0.0f) {
                if (std::abs(inputDir.x) >= std::abs(inputDir.y)) {
                    transf.facing.x = (inputDir.x > 0.0f) ? 1.0f : -1.0f;
                    transf.facing.y = 0.0f;
                }
                else {
                    transf.facing.x = 0.0f;
                    transf.facing.y = (inputDir.y > 0.0f) ? -1.0f : 1.0f;
                }
            }

            if (inputDir.length() > 0.0f) {
                inputDir = inputDir.normalized();
                Vec2 targetVelocity = inputDir * MAX_SPEED;

                transf.velocity.x += (targetVelocity.x - transf.velocity.x) * ACCEL;
                transf.velocity.y += (targetVelocity.y - transf.velocity.y) * ACCEL;
            }
            else {
                transf.velocity.x *= DRAG;
                transf.velocity.y *= DRAG;

                if (std::abs(transf.velocity.x) < 0.05f) {
                    transf.velocity.x = 0.0f;
                }
                if (std::abs(transf.velocity.y) < 0.05f) {
                    transf.velocity.y = 0.0f;
                }
            }

            room = Vec2(
                floor(player->getComponent<CTransform>().position.x / gameEngine->getWidth()),
                floor(player->getComponent<CTransform>().position.y / gameEngine->getHeight())
            );
        }

        if (e->hasComponent<CFollowPlayer>()) {
            CFollowPlayer& follow = e->getComponent<CFollowPlayer>();
            CTransform& transf = e->getComponent<CTransform>();
            CTransform& playerPos = player->getComponent<CTransform>();
            bool blocked = false;
            follow.home = playerPos.position;

            for (auto& e2 : entityManager.getEntities()) {
                if (e2->getComponent<CBoundingBox>().blocksVision) {
                    if (e == e2) continue;
                    if (e2->getID() == "PLAYER") continue;
                    if (e2->getID() == "ENEMY") continue;
                    if (e2->getID() == "SWORD") continue;

                    if (Physics::entityIntersect(transf.position, playerPos.position, e2)) {
                        blocked = true;
                        break;
                    }
                }
            }

            Vec2 delta = follow.home - transf.position;
            float dist = delta.length();

            if (dist > 0.0f && dist < 240.0f && !blocked) {
                transf.velocity = delta.normalized() * follow.speed;
            }
            else {
                delta = follow.base - transf.position;
                dist = delta.length();
                if (dist > 5.0f) {
                    transf.velocity = delta.normalized() * follow.speed;
                }
                else {
					transf.velocity = Vec2(0.0f, 0.0f);
                }
            }
        }

        if (e->hasComponent<CPatrol>()) {
            CPatrol& patrol = e->getComponent<CPatrol>();
            CTransform& transf = e->getComponent<CTransform>();

            Vec2 target = patrol.positions[patrol.currentPosition];
            Vec2 delta = target - transf.position;
            float dist = delta.length();

            if (dist <= patrol.speed) {
                transf.position = target;
                transf.velocity.x = 0.0f;
                transf.velocity.y = 0.0f;
                patrol.currentPosition = (patrol.currentPosition + 1) % patrol.positions.size();
            }
            else {
                transf.velocity = delta.normalized() * patrol.speed;
            }
        }

        if (e->hasComponent<CTransform>()) {
            CTransform& transform = e->getComponent<CTransform>();
            transform.prevPosition = transform.position;
            transform.position += transform.velocity;
        }

        if (e->hasComponent<CInvincibility>()) {
            CInvincibility& inv = e->getComponent<CInvincibility>();
            if (inv.remaining > 0) {
                inv.remaining--;
            }
            else {
                e->removeComponent<CInvincibility>();
            }
        }
    }
}

/**
 * Collision System
 *
 * Checks for any collisions between DYNAMIC objects and any other objects
 *
 */
void ScenePlay::sCollision() {
    for (auto& de : entityManager.getEntities("DYNAMIC")) {
        int topYCollisions = 0;

        // deferred resolution variables
		bool skipPos = false; // skip position resolution if true
        float maxTopPush = 0.0f;    // push-up amount when landing on top of tiles
        float maxBottomPush = 0.0f; // push-down amount when hitting underside of tiles
		float maxLeftPush = 0.0f; // push-left amount when hitting right side of tiles
		float maxRightPush = 0.0f; // push-right amount when hitting left side of tiles
        std::vector<std::shared_ptr<Entity>> topColliders;
        std::vector<std::shared_ptr<Entity>> bottomColliders;
		std::vector<std::shared_ptr<Entity>> leftColliders;
		std::vector<std::shared_ptr<Entity>> rightColliders;

        for (auto& e : entityManager.getEntities()) {
            if (de == e) continue;
            if (de->getID() == "INTERACT" || e->getID() == "INTERACT") skipPos = true;
			if (de->getID() == "INTERACT" || e->getID() == "INTERACT") if (de->getID() != "PLAYER" && e->getID() != "PLAYER") skipPos = true;
            if (e->getTag() == "DEC") continue;

            Vec2 prevCol = Physics::getPreviousOverlap(de, e);
            Vec2 currCol = Physics::getOverlap(de, e);

            if (currCol.x > 0.0f && currCol.y > 0.0f) {

                // -------------------- INTERACTION RESOLUTIONS -------------------- 

                if ((de->getID() == "PLAYER" && e->getID() == "ENEMY") ||
                    (de->getID() == "ENEMY" && e->getID() == "PLAYER")) {
                    auto playerEntity = (de->getID() == "PLAYER") ? de : e;
                    auto enemyEntity = (de->getID() == "ENEMY") ? de : e;

                    // Allow enemies to push player, not the other way around.
                    if (de->getID() == "ENEMY") {
                        continue;
                    }

                    if (!playerEntity->hasComponent<CInvincibility>()) {
                        gameEngine->changeScene("BATTLE", std::make_shared<SceneBattle>(gameEngine, playerEntity, enemyEntity, shared_from_this()));
                    }
                }

                if ((de->getID() == "PLAYER" && e->getID() == "BOSS") ||
                    (de->getID() == "BOSS" && e->getID() == "PLAYER")) {
                    auto playerEntity = (de->getID() == "PLAYER") ? de : e;
                    auto enemyEntity = (de->getID() == "BOSS") ? de : e;

                    // Allow enemies to push player, not the other way around.
                    if (de->getID() == "BOSS") {
                        continue;
                    }

                    if (!playerEntity->hasComponent<CInvincibility>()) {
                        gameEngine->changeScene("BATTLE", std::make_shared<SceneBattle>(gameEngine, playerEntity, enemyEntity, shared_from_this()));
                    }
                }

                if ((de->getID() == "PLAYER" && e->getID() == "EXIT") ||
                    (de->getID() == "EXIT" && e->getID() == "PLAYER")) {
                    auto playerEntity = (de->getID() == "PLAYER") ? de : e;

                    std::string mappath = gameEngine->getAssets().getRandomMap();

                    for (int i = 0; i < 5 && mappath == levelPath; i++) {
                        if (mappath != levelPath) {
                            break;
                        }
                        mappath = gameEngine->getAssets().getRandomMap();
                    }

                    gameEngine->stopMusic("TITLEMUSIC");
                    gameEngine->changeScene("PLAY", std::make_shared<ScenePlay>(gameEngine, mappath, playerEntity, false, stage + 1));
                    return;
                }

                if ((de->getID() == "INTERACT" && e->getID() == "CHEST") ||
                    (de->getID() == "CHEST" && e->getID() == "INTERACT")) {
                    auto interactEntity = (de->getID() == "INTERACT") ? de : e;
                    auto chest = (de->getID() == "CHEST") ? de : e;

                    if (chest->getComponent<CState>().state == "OPEN") {
                        continue;
					}
                    if (chest->hasComponent<CMagic>()) {
                        std::vector<MagicSpec> magicvec = player->getComponent<CMagic>().magic;
                        MagicSpec magic = gameEngine->getAssets().getMagic(chest->getComponent<CMagic>().magic[0].id);
                        auto it = std::find_if(magicvec.begin(), magicvec.end(), [&](const MagicSpec& m) {
                            return m.id == magic.id;
                            });
                        if (it == magicvec.end()) {
                            player->getComponent<CMagic>().magic.push_back(magic);
                            message = "Found " + chest->getComponent<CMagic>().magic[0].name + "!";
                        }
                        else {
                            message = "Found (duplicate) " + chest->getComponent<CMagic>().magic[0].name + "!";
                        }
						chest->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("MAGICCHESTOPEN");
                        gameEngine->playSound("GOTITEM");
						chest->getComponent<CState>().state = "OPEN";
                        showMessage = true;
                    }
                    else if (chest->hasComponent<CItems>()) {
                        chest->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("CHESTOPEN");
                        gameEngine->playSound("GOTITEM");
                        chest->getComponent<CState>().state = "OPEN";
						player->getComponent<CItems>().items.push_back(chest->getComponent<CItems>().items[0]);
                        showMessage = true;
                        message = "Found " + chest->getComponent<CItems>().items[0].name +"!";
                        buildInventoryMenu();
                    }
                    else if (chest->hasComponent<CWeapons>()) {
                        std::vector<WeaponSpec> weaponvec = player->getComponent<CWeapons>().weapons;
                        WeaponSpec weapon = gameEngine->getAssets().getWeapon(chest->getComponent<CWeapons>().weapons[0].id);
                        auto it = std::find_if(weaponvec.begin(), weaponvec.end(), [&](const WeaponSpec& w) {
                            return w.id == weapon.id;
                            });
                        if (it == weaponvec.end()) {
                            player->getComponent<CWeapons>().weapons.push_back(weapon);
                            message = "Found " + chest->getComponent<CWeapons>().weapons[0].name + "!";
                        }
                        else {
                            message = "Found (duplicate) " + chest->getComponent<CWeapons>().weapons[0].name + "!";
                        }
                        chest->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("WEAPONCHESTOPEN");
                        gameEngine->playSound("GOTITEM");
                        chest->getComponent<CState>().state = "OPEN";
                        showMessage = true;
					}
                }

                // -------------------- POSITIONAL RESOLUTIONS --------------------

                bool prevSepX = (prevCol.x <= 0.0f);
                bool prevSepY = (prevCol.y <= 0.0f);

                if (!skipPos) {

                    // horizontal. previous overlap in y direction, but not in x direction
                    if (prevSepX && !prevSepY) {
                        if (de->getComponent<CTransform>().prevPosition.x < e->getComponent<CTransform>().prevPosition.x) {
                            // left
                            if (currCol.x > maxLeftPush) {
                                maxLeftPush = currCol.x;
                            }
                            leftColliders.push_back(e);
                        }
                        else {
                            // right
                            if (currCol.x > maxRightPush) {
                                maxRightPush = currCol.x;
                            }
                            rightColliders.push_back(e);
                        }
                    }
                    // vertical. previous overlap in x direction, but not in y direction
                    else if (prevSepY && !prevSepX) {
                        if (de->getComponent<CTransform>().prevPosition.y < e->getComponent<CTransform>().prevPosition.y) {
                            // on top 
                            if (currCol.y > maxTopPush) {
                                maxTopPush = currCol.y;
                            }
                            topColliders.push_back(e);
                        }
                        else {
                            // below 
                            if (currCol.y > maxBottomPush) {
                                maxBottomPush = currCol.y;
                            }
                            bottomColliders.push_back(e);
                        }
                    }
                    // horizontal and vertical (from corner). no previous overlap in x or y directions and dynamic entity is not grounded to prevent false positives for corner collisions when grounded
                    else if (prevSepX && prevSepY) {
                        if (currCol.x < currCol.y) {
                            if (de->getComponent<CTransform>().prevPosition.x < e->getComponent<CTransform>().prevPosition.x) {
                                if (currCol.x > maxLeftPush) {
                                    maxLeftPush = currCol.x;
                                }
                                leftColliders.push_back(e);
                            }
                            else {
                                if (currCol.x > maxRightPush) {
                                    maxRightPush = currCol.x;
                                }
                                rightColliders.push_back(e);
                            }
                        }
                        else {
                            if (de->getComponent<CTransform>().prevPosition.y < e->getComponent<CTransform>().prevPosition.y) {
                                if (currCol.y > maxTopPush) {
                                    maxTopPush = currCol.y;
                                }
                                topColliders.push_back(e);
                            }
                            else {
                                if (currCol.y > maxBottomPush) {
                                    maxBottomPush = currCol.y;
                                }
                                bottomColliders.push_back(e);
                            }
                        }
                    }
                    else {
                        if (currCol.x < currCol.y) {
                            if (de->getComponent<CTransform>().position.x < e->getComponent<CTransform>().position.x) {
                                if (currCol.x > maxLeftPush) {
                                    maxLeftPush = currCol.x;
                                }
                                leftColliders.push_back(e);
                            }
                            else {
                                if (currCol.x > maxRightPush) {
                                    maxRightPush = currCol.x;
                                }
                                rightColliders.push_back(e);
                            }
                        }
                        else {
                            if (de->getComponent<CTransform>().position.y < e->getComponent<CTransform>().position.y) {
                                if (currCol.y > maxTopPush) {
                                    maxTopPush = currCol.y;
                                }
                                topColliders.push_back(e);
                            }
                            else {
                                if (currCol.y > maxBottomPush) {
                                    maxBottomPush = currCol.y;
                                }
                                bottomColliders.push_back(e);
                            }
                        }
                    }

                }

            }
        }

        // vertical top resolution
        if (maxTopPush > 0.0f) {
            de->getComponent<CTransform>().position.y -= maxTopPush;
        }

        // vertical under/below resolution
        if (maxBottomPush > 0.0f) {
            de->getComponent<CTransform>().position.y += maxBottomPush;
        }

        // horizontal left resolution
        if (maxLeftPush > 0.0f) {
            de->getComponent<CTransform>().position.x -= maxLeftPush;
        }

        // horizontal right resolution
        if (maxRightPush > 0.0f) {
            de->getComponent<CTransform>().position.x += maxRightPush;
        }
    }
}

/**
 * Render System
 * 
 * Renders the textures, using the camera system
 * 
 */
void ScenePlay::sRender() {
    BeginDrawing();
    BeginMode2D(mainCamera);
    ClearBackground(BLACK);

    if (renderTextures) {
        drawVisibleTiledMap();
        renderTex();
    }
    if (renderBoundingBox)
        renderBB();
    if (renderGridLines)
        renderGrid();
    if (renderVisionDebug) {
        renderAIDebug();
    }

    EndMode2D();
    sGUI();
    EndDrawing();
}

/**
 * GUI system
 * 
 * Renders the UI for inventory, messages, and ImGui debug controls
 * 
 */
void ScenePlay::sGUI(){

    // IMGUI DEBUG UI

    if (this->levelPath == "assets/TestRoom.tmj") {

        rlImGuiBegin();
        ImGui::SetNextWindowSize(ImVec2(400, 350));
        ImGui::Begin("Debug", NULL, ImGuiWindowFlags_NoResize);
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Controls"))
            {
                ImGui::SeparatorText("Rendering Controls");
                ImGui::Checkbox("Textures", &renderTextures);
                ImGui::Checkbox("Bounding Boxes", &renderBoundingBox);
                ImGui::Checkbox("Vision Debug", &renderVisionDebug);
                ImGui::Checkbox("Grid", &renderGridLines);
                ImGui::SeparatorText("Camera Controls");
                ImGui::Checkbox("Follow Camera", &followCam);
                /* Room position debug
                ImGui::Text(std::to_string(room.x).c_str());
                ImGui::Text(std::to_string(room.y).c_str());
                */
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Entities"))
            {
                int count = 0;
                if (ImGui::TreeNode("By Tag")) {
                    for (auto& tag : entityManager.getEntityMap()) {
                        if (ImGui::TreeNode(tag.first.c_str())) {
                            for (auto& e : tag.second) {
                                std::string str = "D###" + std::to_string(reinterpret_cast<uintptr_t>(e.get()));
                                if (ImGui::Button(str.c_str())) {
                                    e->destroy();
                                }
                                ImGui::SameLine();
                                std::string text = tag.first + " " + e->getID() + " " + "(" + std::to_string((int)e->getComponent<CTransform>().position.x) + ", " + std::to_string((int)e->getComponent<CTransform>().position.y) + ")";
                                ImGui::Text(text.c_str());
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("All")) {
                    for (auto e : entityManager.getEntities()) {
                        std::string str = "D###" + std::to_string(reinterpret_cast<uintptr_t>(e.get()));
                        if (ImGui::Button(str.c_str())) {
                            e->destroy();
                        }
                        ImGui::SameLine();
                        std::string text = e->getTag() + " " + e->getID() + " " + "(" + std::to_string((int)e->getComponent<CTransform>().position.x) + ", " + std::to_string((int)e->getComponent<CTransform>().position.y) + ")";
                        ImGui::Text(text.c_str());
                    }
                    ImGui::TreePop();
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
        ImGui::End();
        rlImGuiEnd();
    }

        // Message Bar
        if (showMessage) {

            const float fontSize = 28.0f;
            const float spacing = 1.0f;
            const float panelWidth = 380.0f;
            const float panelHeight = 140.0f;
            const float sideMargin = 40.0f;
            const float bottomMargin = 30.0f;
            const float panelY = gameEngine->getHeight() - panelHeight - bottomMargin;
            const float leftPanelX = sideMargin;
            const float rightPanelX = gameEngine->getWidth() - sideMargin - panelWidth;
            const float textPaddingX = 20.0f;
            const float textPaddingY = 18.0f;
            const float lineHeight = 32.0f;

            const float topPanelX = (gameEngine->getWidth() - panelWidth * 3) / 2.0f;
            const float topPanelY = sideMargin;
            Vector2 messageSize = MeasureTextEx(font, message.c_str(), fontSize, spacing);
            const float messagePanelWidth = panelWidth * 3.0f;
            const float messagePanelHeight = panelHeight / 2.0f;

            const float messageX = topPanelX + (messagePanelWidth - messageSize.x) / 2.0f;
            const float messageY = topPanelY + (messagePanelHeight - messageSize.y) / 2.0f;

            DrawTexturePro(
                menuBox,
                Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
                Rectangle{ topPanelX, topPanelY, panelWidth * 3, panelHeight / 2 },
                Vector2{ 0.0f, 0.0f },
                0.0f,
                WHITE
            );

            // TOP MESSAGE TEXT

            DrawTextEx(
                font,
                message.c_str(),
                Vector2(messageX, messageY),
                fontSize,
                spacing,
                BLACK
            );
        }

        // Inventory Screen
        if (inventory) {

            // Sidebar & main box

            const float resultsWidth = gameEngine->getWidth() * 0.40f;
            const float resultsHeight = gameEngine->getHeight() * 0.50f;
            const float resultsX = 30.0f;
            const float resultsY = 30.0f;

            DrawTexturePro(
                invBox,
                Rectangle{ 0.0f, 0.0f, static_cast<float>(invBox.width), static_cast<float>(invBox.height) },
                Rectangle{ resultsX, resultsY, resultsWidth, resultsHeight },
                Vector2{ 0.0f, 0.0f },
                0.0f,
                WHITE
            );

            const float menuFontSize = 28.0f;
            const float menuSpacing = 3.0f;
            const float menuStartX = resultsX + 24.0f;
            const float menuStartY = resultsY + 45.0f;
            const float menuLineHeight = 60.0f;

            for (int i = 0; i < menuStrings.size(); i++) {
                Color textColor = BLACK;
                if (i == selectedMenuItem) {
                    textColor = RED;
                }

                DrawTextEx(
                    font,
                    menuStrings[i].c_str(),
                    Vector2(menuStartX, menuStartY + menuLineHeight * i),
                    menuFontSize,
                    menuSpacing,
                    textColor
                );
            }

            // Sub menu stuff

            // STATUS MENU
            if (selectedMenuItem == 0) {
                const CName& name = player->getComponent<CName>();
                const CStats& stats = player->getComponent<CStats>();
                const CHealth& health = player->getComponent<CHealth>();

                const float sidebarWidth = 190.0f;
                const float contentX = resultsX + sidebarWidth + 20.0f;
                const float contentY = resultsY + 20.0f;

                const float portraitWidth = static_cast<float>(portrait.width * 2);
                const float portraitHeight = static_cast<float>(portrait.height * 2);

                DrawTexturePro(
                    portrait,
                    Rectangle{ 0.0f, 0.0f, static_cast<float>(portrait.width), static_cast<float>(portrait.height) },
                    Rectangle{ contentX, contentY, portraitWidth, portraitHeight },
                    Vector2{ 0.0f, 0.0f },
                    0.0f,
                    WHITE
                );

                const float nameX = contentX + portraitWidth + 20.0f;
                const float nameY = contentY + 8.0f;

                DrawTextEx(
                    font,
                    name.name.c_str(),
                    Vector2(nameX, nameY),
                    32.0f,
                    2.0f,
                    BLACK
                );

                DrawTextEx(
                    font,
                    ("Floor: " + std::to_string(stats.stage)).c_str(),
                    Vector2(nameX + 12.0f, nameY + 45.0f),
                    20.0f,
                    2.0f,
                    BLACK
                );

                const float statsX = contentX;
                const float statsY = contentY + portraitHeight + 20.0f;
                const float lineHeight = 25.0f;
                const float statFontSize = 24.0f;
                const float statSpacing = 2.0f;

                DrawTextEx(font, TextFormat("Level: %i", stats.level), Vector2(statsX, statsY + lineHeight * 0), statFontSize, statSpacing, BLACK);
                DrawTextEx(font, TextFormat("EXP: %i / %i", stats.exp, stats.nextlevel), Vector2(statsX, statsY + lineHeight * 1), statFontSize, statSpacing, BLACK);
                DrawTextEx(font, TextFormat("HP: %i / %i", health.current, health.max), Vector2(statsX, statsY + lineHeight * 2), statFontSize, statSpacing, RED);
                DrawTextEx(font, TextFormat("MP: %i / %i", health.currentMana, health.maxMana), Vector2(statsX, statsY + lineHeight * 3), statFontSize, statSpacing, BLUE);
                DrawTextEx(font, TextFormat("Strength: %i", stats.strength), Vector2(statsX, statsY + lineHeight * 4), statFontSize, statSpacing, BLACK);
                DrawTextEx(font, TextFormat("Defense: %i", stats.defense), Vector2(statsX, statsY + lineHeight * 5), statFontSize, statSpacing, BLACK);
                DrawTextEx(font, TextFormat("Intelligence: %i", stats.intelligence), Vector2(statsX, statsY + lineHeight * 6), statFontSize, statSpacing, BLACK);
                DrawTextEx(font, TextFormat("Magic Def: %i", stats.magicdefense), Vector2(statsX, statsY + lineHeight * 7), statFontSize, statSpacing, BLACK);
                DrawTextEx(font, TextFormat("Speed: %i", stats.speed), Vector2(statsX, statsY + lineHeight * 8), statFontSize, statSpacing, BLACK);
            }

            // ITEMS MENU
            else if (subMenu == 1) {
                const float sidebarWidth = 190.0f;
                const float contentX = resultsX + sidebarWidth + 12.0f;
                const float contentY = resultsY + 20.0f;
                const float contentWidth = resultsWidth - sidebarWidth - 40.0f;

                const float descFontSize = 15.0f;
                const float itemFontSize = 22.0f;
                const float spacing = 1.0f;

                const float descY = contentY + 12.0f;
                const float listY = contentY + 55.0f;
                const float rowHeight = 55.0f;
                const int itemsPerPage = 5;

                if (inventoryMenuItems.empty()) {
                    DrawTextEx(
                        font,
                        "No items!",
                        Vector2(contentX, listY),
                        itemFontSize,
                        spacing,
                        BLACK
                    );
                }
                else {
                    if (selectedSubMenuItem >= inventoryMenuItems.size()) {
                        selectedSubMenuItem = inventoryMenuItems.size() - 1;
                    }

                    selectTip = inventoryMenuItems[selectedSubMenuItem].description;

                    const int startIndex = (selectedSubMenuItem / itemsPerPage) * itemsPerPage;
                    int endIndex = startIndex + itemsPerPage;
                    if (endIndex > inventoryMenuItems.size()) {
                        endIndex = inventoryMenuItems.size();
                    }

                    for (int i = startIndex; i < endIndex; i++) {
                        const int row = i - startIndex;
                        const float textY = listY + row * rowHeight;
                        Color textColor = BLACK;
                        if (subControl) {
                            textColor = (i == selectedSubMenuItem) ? RED : BLACK;

                            DrawTextEx(
                                font,
                                selectTip.c_str(),
                                Vector2(contentX, descY),
                                descFontSize,
                                spacing,
                                BLACK
                            );
                        }

                        DrawTextEx(
                            font,
                            inventoryMenuItems[i].name.c_str(),
                            Vector2(contentX, textY),
                            itemFontSize,
                            spacing,
                            textColor
                        );

                        std::string countText = std::to_string(inventoryMenuCounts[i]);
                        Vector2 countSize = MeasureTextEx(font, countText.c_str(), itemFontSize, spacing);
                        const float countX = contentX + contentWidth - countSize.x - 10.0f;

                        DrawTextEx(
                            font,
                            countText.c_str(),
                            Vector2(countX, textY),
                            itemFontSize,
                            spacing,
                            textColor
                        );
                    }

                    if (startIndex + itemsPerPage < inventoryMenuItems.size()) {
                        const float arrowX = contentX + (contentWidth - static_cast<float>(arrow.width)) / 2.0f;
                        const float arrowY = listY + itemsPerPage * rowHeight;

                        DrawTexture(
                            arrow,
                            static_cast<int>(arrowX),
                            static_cast<int>(arrowY),
                            WHITE
                        );
                    }
                }
            }
            else if (subMenu == 2) {
                const auto& spells = player->getComponent<CMagic>().magic;
                const CHealth& health = player->getComponent<CHealth>();

                const float sidebarWidth = 190.0f;
                const float contentX = resultsX + sidebarWidth + 12.0f;
                const float contentY = resultsY + 30.0f;
                const float contentWidth = resultsWidth - sidebarWidth - 40.0f;

                const float headerFontSize = 14.0f;
                const float itemFontSize = 22.0f;
                const float spacing = 1.0f;

                const float manaY = contentY;
                const float descY = contentY + 28.0f;
                const float listY = contentY + 70.0f;
                const float rowHeight = 55.0f;
                const int itemsPerPage = 5;

                DrawTextEx(
                    font,
                    TextFormat("MP: %i / %i", health.currentMana, health.maxMana),
                    Vector2(contentX + 65.0f, manaY),
                    25.0f,
                    spacing,
                    BLUE
                );

                if (spells.empty()) {
                    selectTip = "No magic";

                    DrawTextEx(
                        font,
                        selectTip.c_str(),
                        Vector2(contentX, descY),
                        headerFontSize,
                        spacing,
                        BLACK
                    );

                    DrawTextEx(
                        font,
                        "No magic",
                        Vector2(contentX, listY),
                        itemFontSize,
                        spacing,
                        BLACK
                    );
                }
                else {
                    if (selectedSubMenuItem >= spells.size()) {
                        selectedSubMenuItem = spells.size() - 1;
                    }

                    selectTip = spells[selectedSubMenuItem].description;

                    const int startIndex = (selectedSubMenuItem / itemsPerPage) * itemsPerPage;
                    int endIndex = startIndex + itemsPerPage;
                    if (endIndex > spells.size()) {
                        endIndex = spells.size();
                    }

                    for (int i = startIndex; i < endIndex; i++) {
                        const int row = i - startIndex;
                        const float textY = listY + row * rowHeight;
                        Color textColor = BLACK;
                        if (subControl) {
                            textColor = (i == selectedSubMenuItem) ? RED : BLACK;

                            DrawTextEx(
                                font,
                                selectTip.c_str(),
                                Vector2(contentX, descY),
                                headerFontSize,
                                spacing,
                                BLACK
                            );
                        }

                        DrawTextEx(
                            font,
                            spells[i].name.c_str(),
                            Vector2(contentX, textY),
                            itemFontSize,
                            spacing,
                            textColor
                        );

                        std::string costText = std::to_string(spells[i].manacost);
                        Vector2 costSize = MeasureTextEx(font, costText.c_str(), itemFontSize, spacing);
                        const float costX = contentX + contentWidth - costSize.x - 10.0f;

                        DrawTextEx(
                            font,
                            costText.c_str(),
                            Vector2(costX, textY),
                            itemFontSize,
                            spacing,
                            textColor
                        );
                    }

                    if (startIndex + itemsPerPage < spells.size()) {
                        const float arrowX = contentX + (contentWidth - static_cast<float>(arrow.width)) / 2.0f;
                        const float arrowY = listY + itemsPerPage * rowHeight;

                        DrawTexture(
                            arrow,
                            static_cast<int>(arrowX),
                            static_cast<int>(arrowY - 25),
                            WHITE
                        );
                    }
                }
            }
            // WEAPONS MENU
            else if (subMenu == 3) {
                const auto& weapons = player->getComponent<CWeapons>().weapons;
                const auto& currentWeapon = player->getComponent<CWeapons>().currentWeapon;

                const float sidebarWidth = 190.0f;
                const float contentX = resultsX + sidebarWidth + 10.0f;
                const float contentY = resultsY + 50.0f;
                const float contentWidth = resultsWidth - sidebarWidth - 40.0f;

                const float headerFontSize = 14.0f;
                const float itemFontSize = 18.0f;
                const float spacing = 1.0f;

                const float descY = contentY;
                const float listY = contentY + 40.0f;
                const float rowHeight = 55.0f;
                const int itemsPerPage = 5;

                if (weapons.empty()) {
                    selectTip = "No weapons";

                    DrawTextEx(
                        font,
                        selectTip.c_str(),
                        Vector2(contentX, descY),
                        headerFontSize,
                        spacing,
                        BLACK
                    );

                    DrawTextEx(
                        font,
                        "No weapons",
                        Vector2(contentX, listY),
                        itemFontSize,
                        spacing,
                        BLACK
                    );
                }
                else {
                    if (selectedSubMenuItem >= weapons.size()) {
                        selectedSubMenuItem = weapons.size() - 1;
                    }

                    selectTip = weapons[selectedSubMenuItem].description;

                    const int startIndex = (selectedSubMenuItem / itemsPerPage) * itemsPerPage;
                    int endIndex = startIndex + itemsPerPage;
                    if (endIndex > weapons.size()) {
                        endIndex = weapons.size();
                    }

                    for (int i = startIndex; i < endIndex; i++) {
                        const int row = i - startIndex;
                        const float textY = listY + row * rowHeight;
                        Color textColor = BLACK;
                        if (subControl) {
                            textColor = (i == selectedSubMenuItem) ? RED : BLACK;

                            DrawTextEx(
                                font,
                                selectTip.c_str(),
                                Vector2(contentX, descY),
                                headerFontSize,
                                spacing,
                                BLACK
                            );
                        }

                        const bool equipped = (weapons[i].id == currentWeapon.id);

                        if (equipped) {
                            DrawTextEx(
                                font,
                                "E",
                                Vector2(contentX, textY),
                                itemFontSize,
                                spacing,
                                textColor
                            );
                        }

                        DrawTextEx(
                            font,
                            weapons[i].name.c_str(),
                            Vector2(contentX + 22.0f, textY),
                            itemFontSize,
                            spacing,
                            textColor
                        );

                        std::string statText = std::to_string(weapons[i].damage) + "  " + weapons[i].effect.id;
                        Vector2 statSize = MeasureTextEx(font, statText.c_str(), itemFontSize, spacing);
                        const float statX = contentX + contentWidth - statSize.x - 10.0f;

                        DrawTextEx(
                            font,
                            statText.c_str(),
                            Vector2(statX, textY),
                            itemFontSize,
                            spacing,
                            textColor
                        );
                    }

                    if (startIndex + itemsPerPage < weapons.size()) {
                        const float arrowX = contentX + (contentWidth - static_cast<float>(arrow.width)) / 2.0f;
                        const float arrowY = listY + itemsPerPage * rowHeight;

                        DrawTexture(
                            arrow,
                            static_cast<int>(arrowX),
                            static_cast<int>(arrowY - 25),
                            WHITE
                        );
                    }
                }
            }
            else if (subMenu == 4) {
                if (subControl) {
                    const float sidebarWidth = 190.0f;
                    const float contentX = resultsX + sidebarWidth + 20.0f;
                    const float contentY = resultsY + 55.0f;

                    const float titleFontSize = 28.0f;
                    const float optionFontSize = 24.0f;
                    const float spacing = 2.0f;
                    const float optionY = contentY + 55.0f;
                    const float optionGap = 40.0f;

                    DrawTextEx(
                        font,
                        "End Run?",
                        Vector2(contentX, contentY),
                        titleFontSize,
                        spacing,
                        BLACK
                    );

                    Color yesColor = BLACK;
                    Color noColor = BLACK;

                    yesColor = (selectedSubMenuItem == 0) ? RED : BLACK;
                    noColor = (selectedSubMenuItem == 1) ? RED : BLACK;

                    DrawTextEx(
                        font,
                        "YES",
                        Vector2(contentX + 10.0f, optionY),
                        optionFontSize,
                        spacing,
                        yesColor
                    );

                    DrawTextEx(
                        font,
                        "NO",
                        Vector2(contentX + 10.0f, optionY + optionGap),
                        optionFontSize,
                        spacing,
                        noColor
                    );
                }
            }

        }
}

/**
 * Render AI Debug information, including vision and patrol paths
 */
void ScenePlay::renderAIDebug(){
    for(auto& e : entityManager.getEntities("DYNAMIC")){
        if(e->hasComponent<CPatrol>()){
            auto patrol=e->getComponent<CPatrol>();
            Vec2 point;
            Vec2 nextPoint;
            for(int i=0;i<patrol.positions.size();i++){
                point=patrol.positions[i];
                int nextIndex=(i+1)%patrol.positions.size();
                nextPoint=patrol.positions[nextIndex];
                DrawLineEx(Vector2(point.x,point.y), Vector2(nextPoint.x,nextPoint.y),2, BLUE);
                DrawCircle(point.x,point.y,7,BLUE);
            }
            
        }
        if(e->hasComponent<CFollowPlayer>()){
            Vec2& home = e->getComponent<CFollowPlayer>().home;
            auto& position=e->getComponent<CTransform>().position;
            Vec2 delta = home - position;
            float dist = delta.length();
            if (dist > 0.0f && dist < 240.0f) {
                DrawLineEx(Vector2(position.x, position.y), Vector2(home.x, home.y), 2, RED);
			}
            else {
                DrawLineEx(Vector2(position.x, position.y), Vector2(home.x, home.y), 2, BLUE);
            }
        }
    }
}

/**
 * Do Action System
 * 
 * Updates the player input and game state based on PRESS or RELEASE actions
 * 
 * @param action action sent from the gameEngine that contains type and name
 */
void ScenePlay::sDoAction(const Action& action) {
    CInput& input = player->getComponent<CInput>();
    CState& state = player->getComponent<CState>();

    if ((action.getType() == "PRESS")) {
        if (action.getName() == "CAMERA") {
            if (followCam) {
                followCam = false;
            }
            else {
                followCam = true;
			}
        }

        // Standard player control

        if (!showMessage && !inventory) {
            if (action.getName() == "UP") {
                input.up = true;
            }
            if (action.getName() == "DOWN") {
                input.down = true;
            }
            if (action.getName() == "LEFT") {
                input.left = true;
            }
            if (action.getName() == "RIGHT") {
                input.right = true;
            }
            if (action.getName() == "INTERACT") {
                if (!state.isInteracting) {
                    input.interact = true;
                    state.isInteracting = true;
                    interact();
                }
            }
            if (action.getName() == "INVENTORY") {
                inventory = true;
                gameEngine->playSound("MENUSELECT");
            }
        }

        // Message on screen control
        else if (showMessage) {
           if (action.getName() == "INTERACT") {
               showMessage = false;
           }
        }

        // Inventory sidebar control
        else if (inventory && !subControl) {
            if (action.getName() == "INVENTORY") {
                inventory = false;
                gameEngine->playSound("BACK");
                selectedMenuItem = 0;
                selectedSubMenuItem = 0;
                subMenu = 0;
			}
            if (action.getName() == "UP") {
                selectedMenuItem--;
				subMenu = selectedMenuItem;
				gameEngine->playSound("MENUSELECT");
                if (selectedMenuItem < 0) {
                    selectedMenuItem = menuStrings.size() - 1;
					subMenu = selectedMenuItem;
                }
            }
            if (action.getName() == "INTERACT") {
                gameEngine->playSound("MENUSELECT");
                subMenu = selectedMenuItem;
                if (selectedMenuItem != 0) {
                    subControl = true;
                    selectedSubMenuItem = 0;

                    if (selectedMenuItem == 1) {
                        buildInventoryMenu();
                    }
                }
            }
            if (action.getName() == "BACK") {
                inventory = false;
                gameEngine->playSound("BACK");
                selectedMenuItem = 0;
                selectedSubMenuItem = 0;
			}
            if (action.getName() == "DOWN") {
                selectedMenuItem++;
				subMenu = selectedMenuItem;
                gameEngine->playSound("MENUSELECT");
                if (selectedMenuItem >= menuStrings.size()) {
                    selectedMenuItem = 0;
					subMenu = selectedMenuItem;
                }
            }
        }

        // Sub menu controls
        else if (subControl) {
            if (action.getName() == "BACK") {
                subControl = false;
                gameEngine->playSound("BACK");
                selectedSubMenuItem = 0;
			}
            if (action.getName() == "INVENTORY") {
                inventory = false;
                subControl = false;
                gameEngine->playSound("BACK");
                selectedMenuItem = 0;
                selectedSubMenuItem = 0;
                subMenu = 0;
            }

			// Sub menu item selection
            if (subMenu == 1) {
                if (action.getName() == "UP") {
                    selectedSubMenuItem--;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem < 0) {
                        selectedSubMenuItem = 0;
                    }
                }

                if (action.getName() == "DOWN") {
                    selectedSubMenuItem++;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem >= inventoryMenuItems.size()) {
                        selectedSubMenuItem = inventoryMenuItems.empty() ? 0 : inventoryMenuItems.size() - 1;
                    }
                }

                if (action.getName() == "INTERACT") {
                    if (!inventoryMenuItems.empty() && selectedSubMenuItem < inventoryMenuItems.size()) {
                        const ItemSpec selectedItem = inventoryMenuItems[selectedSubMenuItem];
                        gameEngine->playSound("MENUSELECT");
                        useItem(selectedItem);
                        refreshInventoryMenuCounts();
                    }
                }
            }

            // Sub menu magic selection
            if (subMenu == 2) {
                const auto& spells = player->getComponent<CMagic>().magic;

                if (action.getName() == "UP") {
                    selectedSubMenuItem--;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem < 0) {
                        selectedSubMenuItem = 0;
                    }
                }
                if (action.getName() == "DOWN") {
                    selectedSubMenuItem++;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem >= spells.size()) {
                        selectedSubMenuItem = spells.size() - 1;
					}
                }
                if (action.getName() == "INTERACT") {
                    if (!spells.empty() && selectedSubMenuItem < spells.size()) {
                        const MagicSpec& selectedSpell = spells[selectedSubMenuItem];
                        gameEngine->playSound("MENUSELECT");
                        useMagic(selectedSpell);
                    }
                }
            }

            // Sub menu weapon selection
            if (subMenu == 3) {
                const auto& weapons = player->getComponent<CWeapons>().weapons;

                if (action.getName() == "UP") {
                    selectedSubMenuItem--;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem < 0) {
                        selectedSubMenuItem = 0;
                    }
                }

                if (action.getName() == "DOWN") {
                    selectedSubMenuItem++;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem >= weapons.size()) {
                        selectedSubMenuItem = weapons.size() - 1;
                    }
                }

                if (action.getName() == "INTERACT") {
                    if (!weapons.empty() && selectedSubMenuItem < weapons.size()) {
                        gameEngine->playSound("MENUSELECT");
                        equipWeapon(selectedSubMenuItem);
                    }
                }
            }

            // Sub menu quit selection
            if (subMenu == 4) {
                if (action.getName() == "UP") {
                    selectedSubMenuItem--;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem < 0) {
                        selectedSubMenuItem = 0;
                    }
                }

                if (action.getName() == "DOWN") {
                    selectedSubMenuItem++;
                    gameEngine->playSound("MENUSELECT");
                    if (selectedSubMenuItem > 1) {
                        selectedSubMenuItem = 1;
                    }
                }

                if (action.getName() == "INTERACT") {
                    gameEngine->playSound("MENUSELECT");

                    if (selectedSubMenuItem == 0) {
                        std::fstream file("HIGHSCORES.txt");
                        std::string str;
                        int num;
                        int highfloor;
                        int highlevel;

                        while (file.good()) {
                            file >> str;
                            if (str == "FLOOR") {
                                file >> num;
                                highfloor = num;
                            }
                            if (str == "LEVEL") {
                                file >> num;
                                highlevel = num;
                            }
                        }

                        file.close();

                        if (player->getComponent<CStats>().stage > highfloor) {
                            highfloor = player->getComponent<CStats>().stage;
                            highlevel = player->getComponent<CStats>().level;

                            std::ofstream outFile("HIGHSCORES.txt", std::ios::trunc);
                            if (outFile.is_open()) {
                                outFile << "FLOOR " << highfloor << '\n';
                                outFile << "LEVEL " << highlevel << '\n';
                            }
                        }
                        gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine));
                    }
                    else {
                        subControl = false;
                        subMenu = selectedMenuItem;
                        selectedSubMenuItem = 0;
                    }
                }
            }
        }
    }

    if ((action.getType() == "RELEASE")) {
        if (action.getName() == "UP") {
            input.up = false;
        }
        if (action.getName() == "DOWN") {
            input.down = false;
        }
        if (action.getName() == "LEFT") {
            input.left = false;
        }
        if (action.getName() == "RIGHT") {
            input.right = false;
        }
        if (action.getName() == "ATTACK") {
            input.attack = false;
        }
    }

    if (action.getType() == "MOUSE_MOVE") {
        input.mouseLocation = action.getPosition();
    }
}

/**
 * Camera System
 * 
 * Updates camera
 * 
 */
void ScenePlay::sCamera(){
    CTransform& transf = player->getComponent<CTransform>();

    if (followCam) {
        mainCamera.target.x = transf.position.x;
        mainCamera.target.y = transf.position.y;
    }
    else {
        int w = gameEngine->getWidth();
        int h = gameEngine->getHeight();
        mainCamera.target.x = room.x * w + w / 2;
        mainCamera.target.y = room.y * h + h / 2;
    }
}

void ScenePlay::battleReturn(std::shared_ptr<Entity> e) {
    
    if (e->getID() == "BOSS") {
        for (const auto& entity : entityManager.getEntities()) {
            if (entity->getID() == "BOSSDOOR") {
                entity->destroy();
            }
        }
    }

    e->destroy();

    buildInventoryMenu();
	CInput& input = player->getComponent<CInput>();
    input.up = false;
    input.down = false;
    input.left = false;
    input.right = false;
	gameEngine->playMusic("TITLEMUSIC");
    player->getComponent<CTransform>().velocity = Vec2(0.0f, 0.0f);
	player->addComponent<CInvincibility>(100);
    player->getComponent<CInvincibility>().remaining = 100;
}

/**
 * Spawns Player
 * 
 * Example of spawning a simple player
 * 
 */
void ScenePlay::spawnPlayer(){
    player=entityManager.addEntity("DYNAMIC", "PLAYER");
    player->addComponent<CState>("DOWN");
    player->addComponent<CInput>();
    player->addComponent<CHealth>(playerConfig.HEALTH, playerConfig.HEALTH);
    player->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("OLSTANDD"),true);
    int scaledHeight=player->getComponent<CAnimation>().animation.getScaledSize().y;
    int scaledWidth=player->getComponent<CAnimation>().animation.getScaledSize().x - 30;
    player->addComponent<CBoundingBox>(Vec2(gameEngine->getTileSizeX(), gameEngine->getTileSizeY()));
    player->addComponent<CTransform>(spawnPoint, Vec2(0.0f,0.0f), 0.0f);
    player->addComponent<CWeapons>();
    player->getComponent<CWeapons>().weapons.push_back(gameEngine->getAssets().getWeapon("ANCIENTBLADE"));
	player->getComponent<CWeapons>().currentWeapon = player->getComponent<CWeapons>().weapons[0];
    
    // Add stats

    CItems& items = player->getComponent<CItems>();

	items.items.push_back(gameEngine->getAssets().getItem("SHEAL"));
    items.items.push_back(gameEngine->getAssets().getItem("SMANA"));

    player->getComponent<CHealth>().maxMana = 10;
    player->getComponent<CHealth>().currentMana = 10;

    player->addComponent<CMagic>();
    CMagic& magic = player->getComponent<CMagic>();
    magic.magic.push_back(gameEngine->getAssets().getMagic("BURN"));
	magic.magic.push_back(gameEngine->getAssets().getMagic("SMEND"));
    magic.magic.push_back(gameEngine->getAssets().getMagic("TOXIN"));
    player->addComponent<CName>("Player");
    player->addComponent<CEffects>();
    player->addComponent<CStats>();
}

void ScenePlay::buildInventoryMenu() {
    inventoryMenuItems.clear();
    inventoryMenuCounts.clear();

    const auto& inventoryItems = player->getComponent<CItems>().items;

    for (const auto& item : inventoryItems) {
        bool found = false;

        for (int i = 0; i < inventoryMenuItems.size(); i++) {
            if (inventoryMenuItems[i].id == item.id) {
                inventoryMenuCounts[i]++;
                found = true;
                break;
            }
        }

        if (!found) {
            inventoryMenuItems.push_back(item);
            inventoryMenuCounts.push_back(1);
        }
    }

    if (selectedSubMenuItem >= inventoryMenuItems.size()) {
        selectedSubMenuItem = inventoryMenuItems.empty() ? 0 : inventoryMenuItems.size() - 1;
    }
}

void ScenePlay::refreshInventoryMenuCounts() {
    const auto& inventoryItems = player->getComponent<CItems>().items;

    for (int i = static_cast<int>(inventoryMenuItems.size()) - 1; i >= 0; i--) {
        int count = 0;

        for (const auto& item : inventoryItems) {
            if (item.id == inventoryMenuItems[i].id) {
                count++;
            }
        }

        if (count == 0) {
            inventoryMenuItems.erase(inventoryMenuItems.begin() + i);
            inventoryMenuCounts.erase(inventoryMenuCounts.begin() + i);

            if (selectedSubMenuItem > i) {
                selectedSubMenuItem--;
            }
            else if (selectedSubMenuItem >= inventoryMenuItems.size() && !inventoryMenuItems.empty()) {
                selectedSubMenuItem = inventoryMenuItems.size() - 1;
            }
            else if (inventoryMenuItems.empty()) {
                selectedSubMenuItem = 0;
            }
        }
        else {
            inventoryMenuCounts[i] = count;
        }
    }
}

void ScenePlay::equipWeapon(int index) {
    CWeapons& equipment = player->getComponent<CWeapons>();

    if (index < 0 || index >= equipment.weapons.size()) {
        gameEngine->playSound("NODAMAGE");
        return;
    }
    if (equipment.weapons[index].id == equipment.currentWeapon.id) {
        gameEngine->playSound("NODAMAGE");
        return;
	}

    WeaponSpec selected = equipment.weapons[index];
    equipment.weapons.erase(equipment.weapons.begin() + index);
    equipment.weapons.insert(equipment.weapons.begin(), selected);
    equipment.currentWeapon = equipment.weapons[0];
    selectedSubMenuItem = 0;
}

void ScenePlay::useItem(const ItemSpec& item) {
    if (item.effect.id == "HEAL") {
        CHealth& health = player->getComponent<CHealth>();
        if (health.current == health.max) {
            gameEngine->playSound("NODAMAGE");
        }
        else {
            health.current += item.effect.magnitude;
            if (health.current > health.max) {
                health.current = health.max;
            }
            gameEngine->playSound("HEAL");
            CItems& items = player->getComponent<CItems>();
            items.removeItem(item);
        }
    }
    else if (item.effect.id == "HEALM") {
        CHealth& health = player->getComponent<CHealth>();
        if (health.currentMana == health.maxMana) {
            gameEngine->playSound("NODAMAGE");
        }
        else {
            health.currentMana += item.effect.magnitude;
            if (health.currentMana > health.maxMana) {
                health.currentMana = health.maxMana;
            }
            gameEngine->playSound("HEAL");
            CItems& items = player->getComponent<CItems>();
            items.removeItem(item);
        }
    }
}

void ScenePlay::useMagic(const MagicSpec& magic) {
    CHealth& health = player->getComponent<CHealth>();
    if (health.currentMana < magic.manacost || magic.effect.type != "RESTORE" || health.current == health.max) {
        gameEngine->playSound("NODAMAGE");
    }
    else {
        health.currentMana -= magic.manacost;
        gameEngine->playSound("HEAL");
        health.current += magic.effect.magnitude;
        if (health.current > health.max) {
            health.current = health.max;
        }
    }
}

void ScenePlay::interact() {
	auto e = entityManager.addEntity("DYNAMIC", "INTERACT");
    e->addComponent<CLifespan>(5);
    e->getComponent<CLifespan>().remaining = 5;

    float bboxSizeX = 16;
    float bboxSizeY = 16;

    CTransform& ptsf = player->getComponent<CTransform>();
    float px = ptsf.position.x;
    float py = ptsf.position.y;
    e->addComponent<CTransform>();
    CTransform& transf = e->getComponent<CTransform>();
    if (ptsf.facing.y == 1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
        transf.position.x = px;
        transf.prevPosition.x = px;
        transf.position.y = py - gameEngine->getTileSizeY();
        transf.prevPosition.y = py - gameEngine->getTileSizeY();
        transf.angle = 0.0f;
    }
    else if (ptsf.facing.y == -1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
        transf.position.x = px;
        transf.prevPosition.x = px;
        transf.position.y = py + gameEngine->getTileSizeY() - 4;
        transf.prevPosition.y = py + gameEngine->getTileSizeY();
        transf.angle = 180.0f;
    }
    else if (ptsf.facing.x == 1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeY, bboxSizeX));
        transf.position.x = px + gameEngine->getTileSizeX() - 4;
        transf.prevPosition.x = px + gameEngine->getTileSizeX() - 4;
        transf.position.y = py;
        transf.prevPosition.y = py;
        transf.angle = 90.0f;
    }
    else if (ptsf.facing.x == -1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeY, bboxSizeX));
        transf.position.x = px - gameEngine->getTileSizeX() + 4;
        transf.prevPosition.x = px - gameEngine->getTileSizeX() + 4;
        transf.position.y = py;
        transf.prevPosition.y = py;
        transf.angle = 270.0f;
    }

    e->getComponent<CBoundingBox>().blocksVision = false;
}


/**
* Teleports the player to a random cave entrance location on the map
*/
void ScenePlay::teleport(std::shared_ptr<Entity> e) {
	std::vector<std::shared_ptr<Entity>> caveEntrances;
    for(auto& entity : entityManager.getEntities("TILE")) {
        if (entity->getID() == "BLACK" && entity->getComponent<CTransform>().position != e->getComponent<CTransform>().position) {
			caveEntrances.push_back(entity);
        }
	}
    if (caveEntrances.empty()) {
        return;
	}

    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, caveEntrances.size() - 1);
    auto choice = caveEntrances[dist(gen)];

	player->getComponent<CTransform>().position = Vec2(choice->getComponent<CTransform>().position.x, choice->getComponent<CTransform>().position.y + gameEngine->getTileSizeY() * 2);
}

/**
 * Gets the world position from room x,y and tile x,y
 * @param rx room x
 * @param ry room y
 * @param tx tile x
 * @param ty tile y
 * return Vec2 with the world position
 */
Vec2 ScenePlay::getPosition(int rx, int ry, int tx, int ty) {
    int w = gameEngine->getTilesX();
    int h = gameEngine->getTilesY();
    return Vec2(rx * w + tx, ry * h + ty);
}


/*
* Selects a random loot item of the specified type and rarity (according to the player's stage) from the game's assets.
* 
* @param type The type of loot to select ("ITEM", "MAGIC", or "WEAPON").
* @return The ID of the selected loot item, or an empty string if no matching items
*/
std::string ScenePlay::selectLoot(std::string type) {
    std::vector<json> matches;
    json j_array;
    if (type == "ITEM") {
        j_array = gameEngine->getAssets().getAllItems();
    }
    else if (type == "MAGIC") {
        j_array = gameEngine->getAssets().getAllMagic();
    }
    else if (type == "WEAPON") {
        j_array = gameEngine->getAssets().getAllWeapons();
    }
    else {
        return "";
    }

    int lootTier;

    if (type == "ITEM") {
        lootTier = std::min(this->stage, 3);
    }
    else {
        lootTier = std::min(this->stage, 5);
    }

    for (const auto& item : j_array) {
        if (item.contains("rarity") && item["rarity"] == lootTier) {
            matches.push_back(item);
        }
    }

    if (matches.empty()) {
        return "";
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, matches.size() - 1);

    return matches[dis(gen)]["id"];
}


/**
* Converts mouse position in window coordinates to world coordinates
*/
Vec2 ScenePlay::mouseToWorld(Vec2 windowPos) {
    //convert to world
    Vector2 worldMouse = GetScreenToWorld2D(Vector2(windowPos.x, windowPos.y), mainCamera);
    return { worldMouse.x,worldMouse.y };
}

/**
* Lifespan system 
* 
* Checks entities' lifespans and removes them if needed.
* 
*/
void ScenePlay::sLifespan() {
    for (auto& e : entityManager.getEntities()) {
        if (e->hasComponent<CLifespan>()) {
            e->getComponent<CLifespan>().remaining--;
            if (e->getComponent<CLifespan>().remaining <= 0) {
                if (e->getID() == "INTERACT") {
                    player->getComponent<CState>().isInteracting = false;
                }
                e->destroy();
            }
        }
    }
}

/**
 * Updates all systems and entityManager
 */
void ScenePlay::update(){
    entityManager.update();

    if (!showMessage && !inventory) {
        sMovement();
        sAnimation();
    }
    
    sCollision();
    sLifespan();
    sMusic();
    sCamera();
    sRender();
    
}