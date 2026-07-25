#include "scenebattle.hpp"
#include "sceneplay.hpp"

SceneBattle::SceneBattle(GameEngine* gameEngine, std::shared_ptr<Entity> player, std::shared_ptr<Entity> enemy, std::shared_ptr<ScenePlay> previousScene) : Scene(gameEngine) {
	this->player = player;
	this->enemies.push_back(enemy);
	this->previousScene = previousScene;
	init();	
}

void SceneBattle::init() {
	entityManager.addExistingEntity(player);
	entityManager.addExistingEntity(enemies[0]);

	menuStrings.push_back("ATTACK");
	menuStrings.push_back("DEFEND");
	menuStrings.push_back("MAGIC");
	menuStrings.push_back("ITEM");
	
	//register input
	registerAction(KEY_D, "RIGHT");
	registerAction(KEY_A, "LEFT");
	registerAction(KEY_W, "UP");
	registerAction(KEY_S, "DOWN");
	registerAction(KEY_SPACE, "SELECT");
	registerAction(KEY_ESCAPE, "QUIT");
	gameEngine->stopMusic("TITLEMUSIC");
	gameEngine->playMusic("BATTLEMUSIC");
	player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLSTANDU");
}

/*
* Battle system during the battle scene. Controls the battle loop.
* 
*/
void SceneBattle::sBattle() {
	if (waitTimer > 0) {
		waitTimer--;
	}

	if (!playerTurn || waitTimer > 0) {
		return;
	}

	if (playerAction == "ATTACK") {
		player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLUSEU");
		gameEngine->playSound("LINKSWING");

		CEquipment& equip = player->getComponent<CEquipment>();

		auto e = entityManager.addEntity("WEAPON", equip.currentWeapon.name);
		e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(equip.currentWeapon.id), true);

		CTransform& transf = e->getComponent<CTransform>();
		transf.position.x = gameEngine->getWidth() * 0.5;
		transf.position.y = gameEngine->getHeight() * 0.65;

		e->addComponent<CLifespan>(10);
		e->getComponent<CLifespan>().remaining = 10;

		auto& enemyHealth = enemies[0]->getComponent<CHealth>();
		enemyHealth.current -= 1;

		if (enemyHealth.current <= 0) {
			enemies[0]->destroy();
			enemies.erase(enemies.begin());

			if (enemies.empty()) {
				gameEngine->changeScene("PLAY", previousScene);
				return;
			}
		}

		playerAction = "";
		playerTurn = false;
		waitTimer = 60;
	}
}

void SceneBattle::sRender() {
	BeginDrawing();

	const Texture2D& background = gameEngine->getAssets().getTexture("BATTLEROOM");
	DrawTexturePro(
		background,
		Rectangle{ 0.0f, 0.0f, static_cast<float>(background.width), static_cast<float>(background.height) },
		Rectangle{ 0.0f, 0.0f, static_cast<float>(gameEngine->getWidth()), static_cast<float>(gameEngine->getHeight()) },
		Vector2{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);

	const float screenWidth = static_cast<float>(gameEngine->getWidth());
	const float screenHeight = static_cast<float>(gameEngine->getHeight());

	auto drawBattleEntity = [](std::shared_ptr<Entity> entity, float x, float y, float scale) {
		if (!entity->hasComponent<CAnimation>()) {
			return;
		}

		auto& transform = entity->getComponent<CTransform>();
		auto& anim = entity->getComponent<CAnimation>();
		const Texture2D& tex = anim.animation.getTexture();

		Rectangle src = anim.animation.getFrameRect();
		if (transform.facing.x != 0) {
			src.width *= transform.facing.x;
		}

		float width = anim.animation.getScaledSize().x * scale;
		float height = anim.animation.getScaledSize().y * scale;

		Rectangle dest = { x, y, width, height };
		Vector2 origin = { width / 2.0f, height / 2.0f };

		DrawTexturePro(tex, src, dest, origin, transform.angle, WHITE);
		};

	const float enemyY = screenHeight * 0.65f;
	const float enemySpacing = 140.0f;
	const float enemyStartX = screenWidth * 0.5f - ((static_cast<float>(enemies.size()) - 1.0f) * enemySpacing * 0.5f);

	for (int i = 0; i < enemies.size(); i++) {
		drawBattleEntity(enemies[i], enemyStartX + i * enemySpacing, enemyY, 1.5f);
	}

	const float playerX = screenWidth * 0.5f;
	const float playerY = screenHeight * 0.85f;
	drawBattleEntity(player, playerX, playerY, 2.0f);

	const Font& font = gameEngine->getAssets().getFont("orbitron");

	const float fontSize = 32.0f;
	const float spacing = 1.0f;
	const float buttonWidth = 180.0f;
	const float buttonHeight = 55.0f;
	const float leftMargin = 40.0f;
	const float topMargin = screenHeight - 170.0f;
	const float columnGap = 20.0f;
	const float rowGap = 18.0f;

	if (playerTurn) {

		for (int i = 0; i < menuStrings.size(); i++) {
			const int col = i % 2;
			const int row = i / 2;

			const float x = leftMargin + col * (buttonWidth + columnGap);
			const float y = topMargin + row * (buttonHeight + rowGap);

			Color fillColor = LIGHTGRAY;
			Color outlineColor = BLACK;
			Color textColor = BLACK;

			if (i == selectedMenuItem) {
				fillColor = RED;
				textColor = WHITE;
			}

			DrawRectangleRounded(
				Rectangle{ x, y, buttonWidth, buttonHeight },
				0.2f,
				8,
				fillColor
			);

			DrawRectangleRoundedLinesEx(
				Rectangle{ x, y, buttonWidth, buttonHeight },
				0.2f,
				8,
				2.0f,
				outlineColor
			);

			Vector2 textSize = MeasureTextEx(font, menuStrings[i].c_str(), fontSize, spacing);
			float textX = x + (buttonWidth - textSize.x) / 2.0f;
			float textY = y + (buttonHeight - textSize.y) / 2.0f;

			DrawTextEx(font, menuStrings[i].c_str(), Vector2(textX, textY), fontSize, spacing, textColor);
		}
	}

	EndDrawing();
}

void SceneBattle::sLifespan() {
	for (auto& e : entityManager.getEntities()) {
		if (e->hasComponent<CLifespan>()) {
			e->getComponent<CLifespan>().remaining--;
			if (e->getComponent<CLifespan>().remaining <= 0) {
				if (e->getID() == "ANCIENTBLADE") {
					player->getComponent<CState>().isAttacking = false;
				}
				e->destroy();
			}
		}
	}
}

void SceneBattle::sMovement() {
	for (auto& e : entityManager.getEntities("WEAPON")) {
		weaponSwing(e, player, entityManager, gameEngine);
	}
}

void SceneBattle::sDoAction(const Action& action) {
	if ((action.getType() == "PRESS")) {
		if (action.getName() == "LEFT") {
			selectedMenuItem--;
			gameEngine->playSound("MENUSELECT");
			if (selectedMenuItem < 0) selectedMenuItem = menuStrings.size() - 1;
		}
		if (action.getName() == "RIGHT") {
			selectedMenuItem++;
			gameEngine->playSound("MENUSELECT");
			if (selectedMenuItem > menuStrings.size() - 1) selectedMenuItem = 0;
		}
		if (action.getName() == "UP") {
			selectedMenuItem -= 2;
			gameEngine->playSound("MENUSELECT");
			if (selectedMenuItem < 0) selectedMenuItem = menuStrings.size() - 1;
		}
		if (action.getName() == "DOWN") {
			selectedMenuItem += 2;
			gameEngine->playSound("MENUSELECT");
			if (selectedMenuItem > menuStrings.size() - 1) selectedMenuItem = 0;
		}	
		if (action.getName() == "SELECT") {
			if (selectedMenuItem == 0) {
				//gameEngine->playSound("ATTACK");
				playerAction = "ATTACK";
			}
			else if (selectedMenuItem == 1) {
				gameEngine->playSound("DEFEND");
				// Implement defend logic here
			}
			else if (selectedMenuItem == 2) {
				gameEngine->playSound("MAGIC");
				// Implement magic logic here
			}
			else if (selectedMenuItem == 3) {
				gameEngine->playSound("ITEM");
				// Implement item logic here
			}
		}
		if (action.getName() == "QUIT") {
			gameEngine->changeScene("PLAY", previousScene);
		}
	}
}

void SceneBattle::sAnimation() {
	for (auto& e : enemies) {
		if (e->hasComponent<CAnimation>()) {
			e->getComponent<CAnimation>().animation.update();
		}
	}
	if (player->hasComponent<CAnimation>()) {
		player->getComponent<CAnimation>().animation.update();
	}
}

void SceneBattle::update() {
	sRender();
	sMusic();
	sBattle();
	sMovement();
	sLifespan();
	sAnimation();
}