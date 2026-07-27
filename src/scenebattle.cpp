#include "scenebattle.hpp"
#include "sceneplay.hpp"

SceneBattle::SceneBattle(GameEngine* gameEngine, std::shared_ptr<Entity> player, std::shared_ptr<Entity> enemy, std::shared_ptr<ScenePlay> previousScene) : Scene(gameEngine) {
	this->player = player;
	this->enemy = enemy;
	this->previousScene = previousScene;
	init();	
}

void SceneBattle::init() {
	entityManager.addExistingEntity(player);
	entityManager.addExistingEntity(enemy);

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
	registerAction(KEY_BACKSPACE, "BACK");
	gameEngine->stopMusic("TITLEMUSIC");
	gameEngine->playMusic("BATTLEMUSIC");
	player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLSTANDU");
	playername = player->getComponent<CName>().name;
}

// ------------ SYSTEM FUNCTIONS ------------

/*
* Battle system during the battle scene. Controls the battle loop.
* 
*/
void SceneBattle::sBattle() {
	if (waitTimer > 0) {
		waitTimer--;
		return;
	}

	switch (battleState) {
	case BattleState::PLAYER_INPUT:
		playerInputState();
		break;

	case BattleState::ENEMY_INPUT:
		enemyInputState();
		break;

	case BattleState::MESSAGE:
		battleState = nextBattleState;
		battleMessage = "";
		break;

	case BattleState::PLAYER_TURN:
		playerTurnState();
		break;

	case BattleState::ENEMY_TURN:
		enemyTurnState();
		break;

	case BattleState::VICTORY:
		victoryState();
		break;
	}
}

void SceneBattle::sRender() {
	BeginDrawing();

	renderUI();

	for (auto e : entityManager.getEntities()) {
		renderBattleEntity(e);
	}

	EndDrawing();
}

void SceneBattle::sLifespan() {
	for (auto& e : entityManager.getEntities()) {
		if (e->hasComponent<CLifespan>()) {
			e->getComponent<CLifespan>().remaining--;
			if (e->getComponent<CLifespan>().remaining <= 0) {
				if (e->getTag() == "WEAPON") {
					player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLSTANDU");
				}
				e->destroy();
			}
		}
	}
}

void SceneBattle::sMovement() {
	for (auto& e : entityManager.getEntities("WEAPON")) {
		battleWeaponSwing(e);
	}

	damageNumber.position += damageNumber.velocity;
	if (damageNumber.remaining <= 0) {
		damageNumber.text = "";
	}
	else {
		damageNumber.remaining--;
	}

}

void SceneBattle::sDoAction(const Action& action) {
	if (battleState != BattleState::PLAYER_INPUT || waitTimer > 0) {
		return;
	}

	// Selection menu
	if ((action.getType() == "PRESS") && menu == 0) {
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
				gameEngine->playSound("MENUSELECT");
				playerAction = "ATTACK";
			}
			else if (selectedMenuItem == 1) {
				gameEngine->playSound("MENUSELECT");
				// Implement defend logic here
			}
			else if (selectedMenuItem == 2) {
				gameEngine->playSound("MENUSELECT");
				// Implement magic logic here
			}
			else if (selectedMenuItem == 3) {
				gameEngine->playSound("MENUSELECT");
				menu = 1;
			}
		}
		if (action.getName() == "QUIT") {
			gameEngine->changeScene("PLAY", previousScene);
		}
	}

	if (action.getType() == "PRESS" && menu == 1) {
		selectedMenuItem = 0;
		std::vector<ItemSpec> uniqueItems;
		std::vector<int> itemCounts;
		const auto& inventory = player->getComponent<CEquipment>().items;

		for (const auto& item : inventory) {
			bool found = false;
			for (int i = 0; i < uniqueItems.size(); i++) {
				if (uniqueItems[i].id == item.id) {
					itemCounts[i]++;
					found = true;
					break;
				}
			}

			if (!found) {
				uniqueItems.push_back(item);
				itemCounts.push_back(1);
			}
		}
		if (action.getName() == "BACK") {
			menu = 0;
			selectedMenuItem = 3;
		}
		if (uniqueItems.empty()) {
			return;
		}
		if (action.getName() == "UP") {
			gameEngine->playSound("MENUSELECT");
			if (selectedMenuItem > 0) {
				selectedMenuItem--;
			}
		}
		if (action.getName() == "DOWN") {
			gameEngine->playSound("MENUSELECT");
			if (selectedMenuItem + 1 < uniqueItems.size()) {
				selectedMenuItem++;
			}
		}
		if (action.getName() == "SELECT" && !uniqueItems.empty()) {
			playerAction = "ITEM";
			itemUsed = uniqueItems[selectedMenuItem];
		}
	}
}

void SceneBattle::sAnimation() {
	for (auto& e : entityManager.getEntities()) {
		if (e->hasComponent<CAnimation>()) {
			e->getComponent<CAnimation>().animation.update();
		}
	}
}

// ----------- HELPER FUNCTIONS --------------

void SceneBattle::renderUI() {
	const Texture2D& background = gameEngine->getAssets().getTexture("BATTLEROOM");
	const Texture2D& menuBox = gameEngine->getAssets().getTexture("MENUBOX");
	DrawTexturePro(
		background,
		Rectangle{ 0.0f, 0.0f, static_cast<float>(background.width), static_cast<float>(background.height) },
		Rectangle{ 0.0f, 0.0f, static_cast<float>(gameEngine->getWidth()), static_cast<float>(gameEngine->getHeight()) },
		Vector2{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);
	const Font& font = gameEngine->getAssets().getFont("alagard");

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
	Vector2 messageSize = MeasureTextEx(font, battleMessage.c_str(), fontSize, spacing);
	const float messagePanelWidth = panelWidth * 3.0f;
	const float messagePanelHeight = panelHeight / 2.0f;

	const float messageX = topPanelX + (messagePanelWidth - messageSize.x) / 2.0f;
	const float messageY = topPanelY + (messagePanelHeight - messageSize.y) / 2.0f;

	// RIGHT PANEL

	DrawTexturePro(
		menuBox,
		Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
		Rectangle{ rightPanelX, panelY, panelWidth, panelHeight },
		Vector2{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);

	DrawTextEx(
		font,
		std::to_string(player->getComponent<CHealth>().current).c_str(),
		Vector2(rightPanelX + textPaddingX, panelY + textPaddingY),
		fontSize,
		spacing,
		BLACK
	);

	// TOP MESSAGE BAR

	if (battleMessage != "") {

		DrawTexturePro(
			menuBox,
			Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
			Rectangle{ topPanelX, topPanelY, panelWidth * 3, panelHeight / 2 },
			Vector2{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);
	}

	// TOP MESSAGE TEXT

	DrawTextEx(
		font,
		battleMessage.c_str(),
		Vector2(messageX, messageY),
		fontSize,
		spacing,
		BLACK
	);

	// SELECTION MENU

	if (battleState == BattleState::PLAYER_INPUT && waitTimer <= 0 && menu == 0) {
		DrawTexturePro(
			menuBox,
			Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
			Rectangle{ leftPanelX, panelY, panelWidth, panelHeight },
			Vector2{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);
		const float leftFontSize = 35.0f;
		const float cellWidth = panelWidth / 2.0f;
		const float cellHeight = panelHeight / 2.0f;

		for (int i = 0; i < menuStrings.size(); i++) {
			const int col = i % 2;
			const int row = i / 2;

			const float cellX = leftPanelX + col * cellWidth;
			const float cellY = panelY + row * cellHeight;

			Color textColor = (i == selectedMenuItem) ? RED : BLACK;

			Vector2 textSize = MeasureTextEx(font, menuStrings[i].c_str(), leftFontSize, spacing);
			const float textX = cellX + (cellWidth - textSize.x) / 2.0f;
			const float textY = cellY + (cellHeight - textSize.y) / 2.0f;

			DrawTextEx(font, menuStrings[i].c_str(), Vector2(textX, textY), leftFontSize, spacing, textColor);
		}
	}

	// ITEM MENU

	if (battleState == BattleState::PLAYER_INPUT && waitTimer <= 0 && menu == 1) {
		DrawTexturePro(
			menuBox,
			Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
			Rectangle{ leftPanelX, panelY, panelWidth, panelHeight },
			Vector2{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);
		std::vector<ItemSpec> uniqueItems;
		std::vector<int> itemCounts;
		const auto& inventory = player->getComponent<CEquipment>().items;

		for (const auto& item : inventory) {
			bool found = false;
			for (int i = 0; i < uniqueItems.size(); i++) {
				if (uniqueItems[i].id == item.id) {
					itemCounts[i]++;
					found = true;
					break;
				}
			}

			if (!found) {
				uniqueItems.push_back(item);
				itemCounts.push_back(1);
			}
		}
		const float itemFontSize = 20.0f;
		const float rowHeight = panelHeight / 4.0f;
		const float rowStartY = panelY + 15.0f;
		const float nameX = leftPanelX + 18.0f;
		const int itemsPerPage = 4;

		if (uniqueItems.empty()) {
			DrawTextEx(font, "No items", Vector2(nameX, panelY + 50.0f), itemFontSize, spacing, BLACK);
		}
		else {
			const int startIndex = (selectedMenuItem / itemsPerPage) * itemsPerPage;
			int endIndex = startIndex + itemsPerPage;
			if (endIndex > uniqueItems.size()) {
				endIndex = uniqueItems.size();
			}

			for (int i = startIndex; i < endIndex; i++) {
				const int row = i - startIndex;
				const float textY = rowStartY + row * rowHeight;
				Color textColor = (i == selectedMenuItem) ? RED : BLACK;

				DrawTextEx(
					font,
					uniqueItems[i].name.c_str(),
					Vector2(nameX, textY),
					itemFontSize,
					spacing,
					textColor
				);

				std::string countText = std::to_string(itemCounts[i]);
				Vector2 countSize = MeasureTextEx(font, countText.c_str(), itemFontSize, spacing);
				const float countX = leftPanelX + panelWidth - 18.0f - countSize.x;

				DrawTextEx(
					font,
					countText.c_str(),
					Vector2(countX, textY),
					itemFontSize,
					spacing,
					textColor
				);
			}
		}
	}
	
	// DAMAGE NUMBERS

	float alpha = static_cast<float>(damageNumber.remaining) / static_cast<float>(damageNumber.total);
	Color tint = damageNumber.color;
	tint.a = static_cast<unsigned char>(255.0f * alpha);

	Vector2 textSize = MeasureTextEx(font, damageNumber.text.c_str(), 40, spacing);
	float textX = damageNumber.position.x - textSize.x / 2.0f;
	float textY = damageNumber.position.y - textSize.y / 2.0f;

	DrawTextEx(font, damageNumber.text.c_str(), Vector2(textX, textY), 40, spacing, tint);
}

void SceneBattle::enemyDie(std::shared_ptr<Entity> e) {
	e->addComponent<CLifespan>(30);
	e->getComponent<CLifespan>().remaining = 30;
	battleState = BattleState::VICTORY;
}

void SceneBattle::battleWeaponSwing(std::shared_ptr<Entity> e) {
	if (!e->hasComponent<CTransform>() || !e->hasComponent<CLifespan>()) {
		return;
	}

	CTransform& weaponTransform = e->getComponent<CTransform>();
	CLifespan& life = e->getComponent<CLifespan>();

	const Vec2 origin(
		gameEngine->getWidth() * 0.5f,
		gameEngine->getHeight() * 0.85f
	);

	const float progress = 1.0f - (static_cast<float>(life.remaining) / static_cast<float>(life.total));
	const float radius = 90.0f;

	const float startAngle = 225.0f;
	const float endAngle = 315.0f;

	const float angle = startAngle + (endAngle - startAngle) * progress;
	const float radians = angle * DEG2RAD;

	weaponTransform.prevPosition = weaponTransform.position;
	weaponTransform.position.x = origin.x + std::cos(radians) * radius;
	weaponTransform.position.y = origin.y + std::sin(radians) * radius;
	weaponTransform.angle = angle + 90.0f;
}

void SceneBattle::renderBattleEntity(std::shared_ptr<Entity> entity) {
	if (!entity->hasComponent<CAnimation>() || !entity->hasComponent<CTransform>()) {
		return;
	}

	float xPos = 0.0f;
	float yPos = 0.0f;
	float scale = 1.0f;

	auto& transform = entity->getComponent<CTransform>();

	Color tint = WHITE;

	if (entity == enemy) {
		if (entity->hasComponent<CLifespan>()) {
			float fade = static_cast<float>(entity->getComponent<CLifespan>().remaining) / static_cast<float>(entity->getComponent<CLifespan>().total);
			tint.a = static_cast<unsigned char>(255.0f * fade);
		}

		xPos = gameEngine->getWidth() * 0.5f;
		yPos = gameEngine->getHeight() * 0.65f;
		scale = 1.5f;
	}
	else if (entity == player) {
		xPos = gameEngine->getWidth() * 0.5f;
		yPos = gameEngine->getHeight() * 0.85f;
		scale = 1.75f;
	}
	else if (entity->getTag() == "WEAPON") {
		xPos = transform.position.x;
		yPos = transform.position.y;
		scale = 1.75f;
	}
	else {
		xPos = transform.position.x;
		yPos = transform.position.y;
	}

	auto& anim = entity->getComponent<CAnimation>();
	const Texture2D& tex = anim.animation.getTexture();

	Rectangle src = anim.animation.getFrameRect();
	if (transform.facing.x != 0) {
		src.width *= transform.facing.x;
	}

	float width = anim.animation.getScaledSize().x * scale;
	float height = anim.animation.getScaledSize().y * scale;

	Rectangle dest = { xPos, yPos, width, height };
	Vector2 origin = { width / 2.0f, height / 2.0f };

	DrawTexturePro(tex, src, dest, origin, transform.angle, tint);
}

void SceneBattle::queueMessage(const std::string& message, BattleState nextState, int frames) {
	battleMessage = message;
	nextBattleState = nextState;
	waitTimer = frames;
	battleState = BattleState::MESSAGE;
}

void SceneBattle::drawDamageNumber(bool playerHit, int damage) {
	DamageNumber number;

	number.total = 45;
	number.remaining = 45;
	number.velocity = Vec2(0.0f, -1.2f);
	number.color = playerHit ? RED : WHITE;
	
	if (damage < 0) {
		number.color = GREEN;
		damage = -damage;
	}

	number.text = std::to_string(damage);

	if (playerHit) {
		number.position = Vec2(
			gameEngine->getWidth() * 0.5f,
			gameEngine->getHeight() * 0.85f - 70.0f
		);
	}
	else {
		number.position = Vec2(
			gameEngine->getWidth() * 0.5f,
			gameEngine->getHeight() * 0.65f - 70.0f
		);
	}

	damageNumber = number;
}

// ----------- BATTLE STATE CONTROL FUNCTIONS --------------

void SceneBattle::playerInputState() {
	bool enemyFaster = enemy->getComponent<CSpeed>().speed > player->getComponent<CSpeed>().speed;
	if (playerAction == "ATTACK") {
		if (enemyFaster) {
			battleState = BattleState::ENEMY_INPUT;
			return;
		}
		queueMessage(playername + " attacks!", BattleState::PLAYER_TURN, 80);
	}
	else if (playerAction == "DEFEND") {
		if (enemyFaster) {
			battleState = BattleState::ENEMY_INPUT;
			return;
		}
		queueMessage(playername + " braces for impact!", BattleState::PLAYER_TURN, 80);
	}
	else if (playerAction == "MAGIC") {
		if (enemyFaster) {
			battleState = BattleState::ENEMY_INPUT;
			return;
		}
		queueMessage(playername + " prepares a spell!", BattleState::PLAYER_TURN, 80);
	}
	else if (playerAction == "ITEM") {
		if (enemyFaster) {
			battleState = BattleState::ENEMY_INPUT;
			return;
		}
		queueMessage(playername + " uses " + itemUsed.name + "!", BattleState::PLAYER_TURN, 80);
	}
}

void SceneBattle::enemyInputState() {
	int usemagic = 0;
	if (enemy->getComponent<CEquipment>().magic.size() > 0) {
		std::mt19937 gen(rd());
		std::bernoulli_distribution dist(0.75);
		usemagic = dist(gen) ? 1 : 0;
		if (usemagic) {
			enemyAction = "MAGIC";
			queueMessage("Enemy casts a spell!", BattleState::ENEMY_TURN, 80);
		}
	}
	else {
		enemyAction = "ATTACK";
		queueMessage("Enemy attacks!", BattleState::ENEMY_TURN, 80);
	}
}

void SceneBattle::playerTurnState() {
	if (playerAction == "ATTACK") {
		player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLUSEU");
		gameEngine->playSound("LINKSWING");

		CEquipment& equip = player->getComponent<CEquipment>();
		auto e = entityManager.addEntity("WEAPON", equip.currentWeapon.name);
		e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(equip.currentWeapon.id), true);
		e->addComponent<CLifespan>(10);
		e->getComponent<CLifespan>().remaining = 10;
		e->addComponent<CTransform>();

		auto& enemyHealth = enemy->getComponent<CHealth>();
		enemyHealth.current -= 1;
		drawDamageNumber(false, 1);

		if (enemyHealth.current <= 0) {
			enemyDie(enemy);
			gameEngine->stopMusic("BATTLEMUSIC");
			gameEngine->playSound("ENEMYDIE");
			queueMessage("Victory! Enemy defeated!", BattleState::VICTORY, 200);
		}
		else {
			playerAction = "";
			if (player->getComponent<CSpeed>().speed < enemy->getComponent<CSpeed>().speed) {
				battleState = BattleState::PLAYER_INPUT;
				waitTimer = 80;
				return;
			}
			battleState = BattleState::ENEMY_INPUT;
			waitTimer = 80;
		}
	}
	if (playerAction == "ITEM") {
		playerAction = "";
		useItem(itemUsed, *this, player);
		menu = 0;
		battleState = BattleState::ENEMY_INPUT;
		waitTimer = 80;
	}
}

void SceneBattle::enemyTurnState() {
	if (enemyAction == "ATTACK") {
		player->getComponent<CHealth>().current -= 1;
		drawDamageNumber(true, 1);
		if (player->getComponent<CHealth>().current <= 0) {
			battleState = BattleState::DEFEAT;
			waitTimer = 200;
		}
		else {
			if (player->getComponent<CSpeed>().speed < enemy->getComponent<CSpeed>().speed) {
				if (playerAction == "ATTACK") {
					queueMessage(playername + " attacks!", BattleState::PLAYER_TURN, 80);
				}
				else if (playerAction == "DEFEND") {
					queueMessage(playername + " braces for impact!", BattleState::PLAYER_TURN, 80);
				}
				else if (playerAction == "MAGIC") {
					queueMessage(playername + " prepares a spell!", BattleState::PLAYER_TURN, 80);
				}
				else if (playerAction == "ITEM") {
					queueMessage(playername + " uses " + itemUsed.name + "!", BattleState::PLAYER_TURN, 80);
				}
			}
			else {
				waitTimer = 80;
				battleState = BattleState::PLAYER_INPUT;
			}
		}
	}
	else if (enemyAction == "MAGIC") {
		gameEngine->playSound("ENEMYMAGIC");
		player->getComponent<CHealth>().current -= 2;
		if (player->getComponent<CHealth>().current <= 0) {
			battleState = BattleState::DEFEAT;
			waitTimer = 200;
		}
		else {
			waitTimer = 80;
			if (player->getComponent<CSpeed>().speed < enemy->getComponent<CSpeed>().speed) {
				battleState = BattleState::PLAYER_INPUT;
			} else {
				battleState = BattleState::PLAYER_INPUT;
			}
		}
	}
}

void SceneBattle::victoryState() {
	previousScene->battleReturn(enemy);
	gameEngine->changeScene("PLAY", previousScene);
}

void SceneBattle::update() {
	entityManager.update();

	sBattle();
	sMovement();
	sLifespan();
	sAnimation();
	sMusic();
	sRender();
}