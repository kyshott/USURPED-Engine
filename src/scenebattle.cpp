#include "scenebattle.hpp"
#include "sceneplay.hpp"

SceneBattle::SceneBattle(GameEngine* gameEngine, std::shared_ptr<Entity> player, std::shared_ptr<Entity> enemy, std::shared_ptr<ScenePlay> previousScene) : Scene(gameEngine) {
	this->player = player;
	this->enemy = enemy;
	this->previousScene = previousScene;
	init();	
}

void SceneBattle::init() {

	enemyFaster = enemy->getComponent<CStats>().speed > player->getComponent<CStats>().speed;

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
	//gameEngine->playMusic("BATTLEMUSIC");
	player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLSTANDU");
	playername = player->getComponent<CName>().name;
	enemyname = enemy->getComponent<CName>().name;

}

// ------------ SYSTEM FUNCTIONS ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

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

	case BattleState::INPUT:
		inputState();
		break;

	case BattleState::ACTION:
		actionState();
		break;

	case BattleState::MESSAGE:
		battleMessage = nextMessage;
		battleState = nextBattleState;
		waitTimer = 80;
		break;

	case BattleState::EFFECTS:
		battleMessage = "";
		sStatusEffects();
		break;

	case BattleState::VICTORY:
		victoryState();
		break;

	case BattleState::RESULTS:
		resultsState();
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
				if (e->getTag() == "MAGIC") {
					if (e->getID() == magicUsed.effect.id) {
						player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLSTANDU");
					}
				}
				e->destroy();
			}
		}
	}

	// Damage number lifespan system
	// Its a UI element... so it doesn't really count as an entity

	if (damageNumber.remaining <= 0) {
		damageNumber.text = "";
	}
	else {
		damageNumber.remaining--;
	}
}

void SceneBattle::sMovement() {
	for (auto& e : entityManager.getEntities("WEAPON")) {
		battleWeaponSwing(e);
	}

	damageNumber.position += damageNumber.velocity;
}

void SceneBattle::sDoAction(const Action& action) {
	if (battleState != BattleState::INPUT && battleState != BattleState::RESULTS || waitTimer > 0) {
		return;
	}

	if (action.getType() == "PRESS") {

		// Selection menu
		if (menu == 0) {
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
					gameEngine->playSound("MENUSELECT");
					playerAction = "ATTACK";
				}
				else if (selectedMenuItem == 1) {
					gameEngine->playSound("MENUSELECT");
					playerAction = "DEFEND";
					selectedMenuItem = 0;
				}
				else if (selectedMenuItem == 2) {
					gameEngine->playSound("MENUSELECT");
					menu = 2;
					selectedMenuItem = 0;
				}
				else if (selectedMenuItem == 3) {
					gameEngine->playSound("MENUSELECT");
					menu = 1;
					selectedMenuItem = 0;
				}
			}
			if (action.getName() == "QUIT") {
				gameEngine->changeScene("PLAY", previousScene);
			}
		}

		// ITEM MENU
		else if (menu == 1) {
			std::vector<ItemSpec> uniqueItems;
			std::vector<int> itemCounts;
			const auto& inventory = player->getComponent<CItems>().items;

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
				gameEngine->playSound("BACK");
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
				gameEngine->playSound("MENUSELECT");
				playerAction = "ITEM";
				itemUsed = uniqueItems[selectedMenuItem];
			}
		}

		// MAGIC MENU
		else if (menu == 2) {
			const auto& spells = player->getComponent<CMagic>().magic;

			if (action.getName() == "BACK") {
				gameEngine->playSound("BACK");
				menu = 0;
				selectedMenuItem = 2;
			}
			if (spells.empty()) {
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
				if (selectedMenuItem + 1 < spells.size()) {
					selectedMenuItem++;
				}
			}
			if (action.getName() == "SELECT" && !spells.empty()) {
				gameEngine->playSound("MENUSELECT");
				playerAction = "MAGIC";
				magicUsed = spells[selectedMenuItem];
			}
		}
		if (battleState == BattleState::RESULTS) {
			if (action.getName() == "SELECT") {
				previousScene->battleReturn(enemy);
				gameEngine->changeScene("PLAY", previousScene);
			}
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

void SceneBattle::sStatusEffects() {
	for (auto& e : entityManager.getEntities()) {
		if (e->hasComponent<CEffects>()) {
			auto& statusEffects = e->getComponent<CEffects>().effects;
			for (auto it = statusEffects.begin(); it != statusEffects.end();) {
				applyEffect(e, *it);
				it->duration--;
				if (it->duration <= 0) {
					it = statusEffects.erase(it);
				}
				else {
					++it;
				}
			}
		}
	}
	if (enemy->getComponent<CHealth>().current <= 0) {
		enemy->addComponent<CLifespan>(45);
		enemy->getComponent<CLifespan>().remaining = 45;
		gameEngine->stopMusic("BATTLEMUSIC");
		gameEngine->playSound("ENEMYDIE");
		queueMessage("Victory!", BattleState::VICTORY);
		collectLoot(player, enemy);
		return;
	}

	if (player->getComponent<CHealth>().current <= 0) {
		battleState = BattleState::DEFEAT;
		waitTimer = 200;
		return;
	}

	battleState = BattleState::INPUT;
	waitTimer = battlespeed;
}

// ----------- BATTLE STATE CONTROL FUNCTIONS ---------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

void SceneBattle::inputState() {

	battleMessage = "";

	if (playerDefended) {
		playerDefended = false;
		player->getComponent<CStats>().defense /= 2;
	}

	// Player has decided, now enemy decides what to do. If the enemy is faster, it will go first.
	if (playerAction != "") {
		// Enemy decision logic
		int usemagic = 0;
		if (enemy->hasComponent<CMagic>()) {
			std::mt19937 gen(rd());
			std::bernoulli_distribution dist(enemy->getComponent<CMagic>().magic[0].castChance);
			usemagic = dist(gen) ? 1 : 0;
			if (usemagic) {
				enemyAction = "MAGIC";
				enemyMagic = enemy->getComponent<CMagic>().magic[0]; // Just use the first spell for now. Need more advanced AI later
			}
		}
		else {
			enemyAction = "ATTACK";
		}

		// Evaluate next turn based on speed stats
		if (enemyFaster && playerAction != "DEFEND") {
			playerTurn = false;
			if (enemyAction == "MAGIC") {
				queueMessage("Enemy casts a spell!", BattleState::ACTION);
			}
			else {
				queueMessage(enemyname + " attacks!", BattleState::ACTION);
			}
		}
		else {
			playerTurn = true;
			if (playerAction == "ATTACK") {
				queueMessage(playername + " attacks!", BattleState::ACTION);
			}
			else if (playerAction == "DEFEND") {
				queueMessage(playername + " defends!", BattleState::ACTION);
			}
			else if (playerAction == "MAGIC") {
				queueMessage(playername + " casts " + magicUsed.name + "!", BattleState::ACTION);
			}
			else if (playerAction == "ITEM") {
				queueMessage(playername + " uses " + itemUsed.name + "!", BattleState::ACTION);
			}
		}

	}
	// Wait override for the first turn of the battle

	waitTimer = 0;
}

void SceneBattle::actionState() {
	battleMessage = "";

	if (playerTurn) {
		playerAct();
	}
	else {
		enemyAct();
	}

	waitTimer = battlespeed;
}


void SceneBattle::playerAct() {
	if (playerAction == "ATTACK") {
		player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLUSEU");
		
		spawnWeapon();

		applyDamage(player, enemy);

		if (enemy->getComponent<CHealth>().current <= 0) {
			enemy->addComponent<CLifespan>(45);
			enemy->getComponent<CLifespan>().remaining = 45;
			gameEngine->stopMusic("BATTLEMUSIC");
			gameEngine->playSound("ENEMYDIE");
			queueMessage("Victory!", BattleState::VICTORY);
			collectLoot(player, enemy);
			return;
		}
		playerAction = "";
	}
	if (playerAction == "DEFEND") {
		player->getComponent<CStats>().defense *= 2;

		playerDefended = true;

		// maybe some sort of defense anim or something

		playerAction = "";
	}
	if (playerAction == "MAGIC") {
		if (player->getComponent<CHealth>().currentMana >= magicUsed.manacost) {
			player->getComponent<CAnimation>().animation = gameEngine->getAssets().getAnimation("OLUSEU");
		}
		applyMagic(player, enemy, magicUsed);

		if (enemy->getComponent<CHealth>().current <= 0) {
			enemy->addComponent<CLifespan>(45);
			enemy->getComponent<CLifespan>().remaining = 45;
			gameEngine->stopMusic("BATTLEMUSIC");
			gameEngine->playSound("ENEMYDIE");
			queueMessage("Victory!", BattleState::VICTORY);
			collectLoot(player, enemy);
			return;
		}
		menu = 0;
		selectedMenuItem = 0;
		playerAction = "";
	}
	if (playerAction == "ITEM") {
		menu = 0;
		playerAction = "";
		useItem();
		CItems& items = player->getComponent<CItems>();
		items.removeItem(itemUsed);
	}
	
	playerTurn = false;
	
	if (enemyFaster && !playerDefended) {
		waitTimer = battlespeed;
		if (!player->getComponent<CEffects>().effects.empty() || !enemy->getComponent<CEffects>().effects.empty()) {
			battleState = BattleState::EFFECTS;
		}
		else {
			battleState = BattleState::INPUT;
		}
	}
	else {
		if (enemyAction == "MAGIC") {
			queueMessage("Enemy casts a spell!", BattleState::ACTION);
		}
		else {
			queueMessage(enemyname + " attacks!", BattleState::ACTION);
		}
	}
}

void SceneBattle::enemyAct() {
	if (enemyAction == "ATTACK") {
		applyDamage(enemy, player);

		if (player->getComponent<CHealth>().current <= 0) {
			battleState = BattleState::DEFEAT;
			waitTimer = 200;
		}

		enemyAction = "";
	}
	else if (enemyAction == "MAGIC") {
		applyMagic(enemy, player, enemyMagic);

		if (enemy->getComponent<CHealth>().current <= 0) {
			enemy->addComponent<CLifespan>(45);
			enemy->getComponent<CLifespan>().remaining = 45;
			gameEngine->stopMusic("BATTLEMUSIC");
			gameEngine->playSound("ENEMYDIE");
			queueMessage("Victory!", BattleState::VICTORY);
			collectLoot(player, enemy);
			return;
		}

		enemyAction = "";
	}

	playerTurn = true;

	if (enemyFaster && !playerDefended) {
		if (playerAction == "ATTACK") {
			queueMessage(playername + " attacks!", BattleState::ACTION);
		}
		else if (playerAction == "DEFEND") {
			queueMessage(playername + " defends!", BattleState::ACTION);
		}
		else if (playerAction == "MAGIC") {
			queueMessage(playername + " casts " + magicUsed.name + "!", BattleState::ACTION);
		}
		else if (playerAction == "ITEM") {
			queueMessage(playername + " uses " + itemUsed.name + "!", BattleState::ACTION);
		}
	}
	else {
		waitTimer = battlespeed;
		if (!player->getComponent<CEffects>().effects.empty() || !enemy->getComponent<CEffects>().effects.empty()) {
			battleState = BattleState::EFFECTS;
		}
		else {
			battleState = BattleState::INPUT;
		}
	}
}

void SceneBattle::victoryState() {
	battleState = BattleState::RESULTS;
	waitTimer = battlespeed;
}

void SceneBattle::resultsState() {
	battleMessage = "";
	//previousScene->battleReturn(enemy);
	//gameEngine->changeScene("PLAY", previousScene);
}

// ----------- HELPER FUNCTIONS -----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

void SceneBattle::renderUI() {
	const Texture2D& background = gameEngine->getAssets().getTexture("BATTLEROOM");
	const Texture2D& menuBox = gameEngine->getAssets().getTexture("MENUBOX");
	const Texture2D& arrow = gameEngine->getAssets().getTexture("ARROW");
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

	if (enemy->getComponent<CHealth>().current > 0) {

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
			player->getComponent<CName>().name.c_str(),
			Vector2(rightPanelX + (panelWidth - MeasureTextEx(font, player->getComponent<CName>().name.c_str(), fontSize, spacing).x) / 2.0f, 
			panelY + textPaddingY),
			fontSize,
			spacing,
			BLACK
		);

		std::string resource = ("HP: " + std::to_string(player->getComponent<CHealth>().current) + "/" + std::to_string(player->getComponent<CHealth>().max)).c_str();

		DrawTextEx(
			font,
			resource.c_str(),
			Vector2(rightPanelX + (panelWidth - MeasureTextEx(font, resource.c_str(), fontSize, spacing).x) / 2.0f, 
			panelY + textPaddingY + 40.0f),
			fontSize,
			spacing,
			RED
		);

		resource = ("MP: " + std::to_string(player->getComponent<CHealth>().currentMana) + "/" + std::to_string(player->getComponent<CHealth>().maxMana));

		DrawTextEx(
			font,
			resource.c_str(),
			Vector2(rightPanelX + (panelWidth - MeasureTextEx(font, resource.c_str(), fontSize, spacing).x) / 2.0f, 
			panelY + textPaddingY + 70.0f),
			fontSize,
			spacing,
			BLUE
		);
	}


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

		// TOP MESSAGE TEXT

		DrawTextEx(
			font,
			battleMessage.c_str(),
			Vector2(messageX, messageY),
			fontSize,
			spacing,
			BLACK
		);
	}

	// SELECTION TIP PANEL

	if (battleState == BattleState::INPUT && waitTimer <= 0 && (menu == 1 || menu == 2)) {
		DrawTexturePro(
			menuBox,
			Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
			Rectangle{ leftPanelX, panelY - 40, panelWidth, panelHeight },
			Vector2{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);

		DrawTextEx(
			font,
			selectTip.c_str(),
			Vector2(leftPanelX + textPaddingX, panelY - 40 + textPaddingY),
			15.0f,
			spacing,
			BLACK
		);
	}

	// SELECTION MENU

	if (battleState == BattleState::INPUT && waitTimer <= 0 && menu == 0) {

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

	if (battleState == BattleState::INPUT && waitTimer <= 0 && menu == 1) {
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
		const auto& inventory = player->getComponent<CItems>().items;

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
		const int itemsPerPage = 3;

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

				selectTip = uniqueItems[selectedMenuItem].description;

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

			if (startIndex + itemsPerPage < uniqueItems.size()) {
				const float arrowX = leftPanelX + (panelWidth - static_cast<float>(arrow.width)) / 2.0f;
				const float arrowY = panelY + panelHeight - static_cast<float>(arrow.height) - 8.0f;

				DrawTexture(
					arrow,
					static_cast<int>(arrowX),
					static_cast<int>(arrowY),
					WHITE
				);
			}
		}
	}

	// MAGIC MENU
	if (battleState == BattleState::INPUT && waitTimer <= 0 && menu == 2) {
		DrawTexturePro(
			menuBox,
			Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
			Rectangle{ leftPanelX, panelY, panelWidth, panelHeight },
			Vector2{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);
		const auto& spells = player->getComponent<CMagic>().magic;

		const float itemFontSize = 20.0f;
		const float rowHeight = panelHeight / 4.0f;
		const float rowStartY = panelY + 15.0f;
		const float nameX = leftPanelX + 18.0f;
		const int itemsPerPage = 3;

		if (spells.empty()) {
			DrawTextEx(font, "No items", Vector2(nameX, panelY + 50.0f), itemFontSize, spacing, BLACK);
		}
		else {
			const int startIndex = (selectedMenuItem / itemsPerPage) * itemsPerPage;
			int endIndex = startIndex + itemsPerPage;
			if (endIndex > spells.size()) {
				endIndex = spells.size();
			}

			for (int i = startIndex; i < endIndex; i++) {
				const int row = i - startIndex;
				const float textY = rowStartY + row * rowHeight;
				Color textColor = (i == selectedMenuItem) ? RED : BLACK;

				selectTip = spells[selectedMenuItem].description;

				DrawTextEx(
					font,
					spells[i].name.c_str(),
					Vector2(nameX, textY),
					itemFontSize,
					spacing,
					textColor
				);

				std::string costText = std::to_string(spells[i].manacost);
				Vector2 countSize = MeasureTextEx(font, costText.c_str(), itemFontSize, spacing);
				const float countX = leftPanelX + panelWidth - 18.0f - countSize.x;

				DrawTextEx(
					font,
					costText.c_str(),
					Vector2(countX, textY),
					itemFontSize,
					spacing,
					textColor
				);
			}

			if (startIndex + itemsPerPage < spells.size()) {
				const float arrowX = leftPanelX + (panelWidth - static_cast<float>(arrow.width)) / 2.0f;
				const float arrowY = panelY + panelHeight - static_cast<float>(arrow.height) - 8.0f;

				DrawTexture(
					arrow,
					static_cast<int>(arrowX),
					static_cast<int>(arrowY),
					WHITE
				);
			}
		}
	}

	// RESULTS SCREEN

	if (battleState == BattleState::RESULTS && waitTimer <= 0) {
		const float resultsWidth = gameEngine->getWidth() * 0.75f;
		const float resultsHeight = gameEngine->getHeight() * 0.60f;
		const float resultsX = (gameEngine->getWidth() - resultsWidth) / 2.0f;
		const float resultsY = (gameEngine->getHeight() - resultsHeight - 100) / 2.0f;

		DrawTexturePro(
			menuBox,
			Rectangle{ 0.0f, 0.0f, static_cast<float>(menuBox.width), static_cast<float>(menuBox.height) },
			Rectangle{ resultsX, resultsY, resultsWidth, resultsHeight },
			Vector2{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);

		DrawTextEx(
			font,
			"SPOILS",
			Vector2(
				resultsX + (resultsWidth - MeasureTextEx(font, "SPOILS", 40.0f, spacing).x) / 2.0f,
				resultsY + resultsHeight - MeasureTextEx(font, "SPOILS", 40.0f, spacing).y - 380.0f
			),
			40.0f,
			spacing,
			BLACK
		);

		std::string resultsText = "Obtained " + std::to_string(enemy->getComponent<CLoot>().gold) + " gold";

		DrawTextEx(
			font,
			resultsText.c_str(),
			Vector2(
				resultsX + (resultsWidth - MeasureTextEx(font, resultsText.c_str(), 28.0f, spacing).x) / 2.0f,
				resultsY + resultsHeight - MeasureTextEx(font, resultsText.c_str(), 28.0f, spacing).y - 300.0f
			),
			28.0f,
			spacing,
			BLACK
		);

		resultsText = "Earned " + std::to_string(enemy->getComponent<CLoot>().exp) + " EXP";

		DrawTextEx(
			font,
			resultsText.c_str(),
			Vector2(
				resultsX + (resultsWidth - MeasureTextEx(font, resultsText.c_str(), 28.0f, spacing).x) / 2.0f,
				resultsY + resultsHeight - MeasureTextEx(font, resultsText.c_str(), 28.0f, spacing).y - 220.0f
			),
			28.0f,
			spacing,
			BLACK
		);

		if (itemDrop != 0) {
			if (itemDrop == 1) {
				resultsText = "Dropped weapon: " + itemDropName;
			}
			else if (itemDrop == 2) {
				resultsText = "Dropped spell: " + itemDropName;
			}
			else if (itemDrop == 3) {
				resultsText = "Dropped item: " + itemDropName;
			}
		}
		else {
			resultsText = "No items dropped";
		}

		DrawTextEx(
			font,
			resultsText.c_str(),
			Vector2(
				resultsX + (resultsWidth - MeasureTextEx(font, resultsText.c_str(), 28.0f, spacing).x) / 2.0f,
				resultsY + resultsHeight - MeasureTextEx(font, resultsText.c_str(), 28.0f, spacing).y - 140.0f
			),
			28.0f,
			spacing,
			BLACK
		);

		if (levelUp) {
			resultsText = "Level Up!";
			DrawTextEx(
				font,
				resultsText.c_str(),
				Vector2(
					resultsX + (resultsWidth - MeasureTextEx(font, resultsText.c_str(), 38.0f, spacing).x) / 2.0f,
					resultsY + resultsHeight - MeasureTextEx(font, resultsText.c_str(), 38.0f, spacing).y - 60.0f
				),
				38.0f,
				spacing,
				GOLD
			);
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

void SceneBattle::useItem() {
	if (itemUsed.effect.id == "HEAL") {
		CHealth& health = player->getComponent<CHealth>();
		health.current += itemUsed.effect.magnitude;
		if (health.current > health.max) {
			health.current = health.max;
		}
		spawnItem(player, itemUsed);
		drawDamageNumber(true, -itemUsed.effect.magnitude, GREEN);
		gameEngine->playSound("HEAL");
	}
	else if (itemUsed.effect.id == "HEALM") {
		CHealth& health = player->getComponent<CHealth>();
		health.currentMana += itemUsed.effect.magnitude;
		if (health.currentMana > health.maxMana) {
			health.currentMana = health.maxMana;
		}
		spawnItem(player, itemUsed);
		drawDamageNumber(true, -itemUsed.effect.magnitude, BLUE);
		gameEngine->playSound("HEAL");
	}
	// It will always kick back to input at some point after using the item, so reset the menu selection to the first item for next time
	selectedMenuItem = 0;
}

void SceneBattle::collectLoot(std::shared_ptr<Entity> looter, std::shared_ptr<Entity> looted) {
	CLoot& loot = looted->getComponent<CLoot>();
	CStats& stats = looter->getComponent<CStats>();

	std::mt19937 gen(rd());
	std::bernoulli_distribution dist(loot.itemDropChance);

	if (dist(gen)) {
		if (loot.lootItem.first == "WEAPON") {
			WeaponSpec weapon = gameEngine->getAssets().getWeapon(loot.lootItem.second);
			itemDropName = weapon.name;
			itemDrop = 1;
			looter->getComponent<CWeapons>().weapons.push_back(weapon);
		}
		else if (loot.lootItem.first == "MAGIC") {
			MagicSpec magic = gameEngine->getAssets().getMagic(loot.lootItem.second);
			itemDropName = magic.name;
			itemDrop = 2;
			std::vector<MagicSpec> magicvec = looter->getComponent<CMagic>().magic;
			auto it = std::find_if(magicvec.begin(), magicvec.end(), [&](const MagicSpec& m) {
				return m.id == magic.id;
				});
			if (it == magicvec.end()) {
				looter->getComponent<CMagic>().magic.push_back(magic);
			}
		}
		else if (loot.lootItem.first == "ITEM") {
			ItemSpec item = gameEngine->getAssets().getItem(loot.lootItem.second);
			itemDropName = item.name;
			itemDrop = 3;
			looter->getComponent<CItems>().items.push_back(item);
		}

		looter->getComponent<CItems>().gold += loot.gold;

		stats.exp += loot.exp;
		if (stats.exp >= stats.nextlevel) {
			levelUp = true;
			stats.level++;
			stats.exp -= stats.nextlevel;
			stats.nextlevel = static_cast<int>(stats.nextlevel * 1.5f);
			stats.speed += 1;
			stats.defense += 1;
			stats.intelligence += 1;
			stats.strength += 1;
			stats.magicdefense += 1;
			CHealth& health = looter->getComponent<CHealth>();
			health.max += 5;
			health.maxMana += 5;
			health.current += 5;
			health.currentMana += 5;
		}
	}
}

void SceneBattle::spawnWeapon() {
	CWeapons& weapons = player->getComponent<CWeapons>();
	auto e = entityManager.addEntity("WEAPON", weapons.currentWeapon.name);
	e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(weapons.currentWeapon.id), true);
	e->addComponent<CLifespan>(15);
	e->getComponent<CLifespan>().remaining = 15;
	e->addComponent<CTransform>();
}

void SceneBattle::spawnEffect(std::shared_ptr<Entity> target, std::string effectId) {
	auto e = entityManager.addEntity("EFFECT", effectId);
	e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(effectId), true);
	e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(effectId), true);
	e->addComponent<CLifespan>(25);
	e->getComponent<CLifespan>().remaining = 25;
	e->addComponent<CTransform>(Vec2(target->getComponent<CTransform>().battlePos.x, target->getComponent<CTransform>().battlePos.y), Vec2(0.0f, 0.0f), 0.0f);
}

void SceneBattle::spawnSpell(std::shared_ptr<Entity> target, MagicSpec spell) {
	auto e = entityManager.addEntity("MAGIC", spell.effect.id);
    e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(spell.effect.id), true);
	e->addComponent<CLifespan>(25);
	e->getComponent<CLifespan>().remaining = 25;
	e->addComponent<CTransform>(Vec2(target->getComponent<CTransform>().battlePos.x, target->getComponent<CTransform>().battlePos.y), Vec2(0.0f, 0.0f), 0.0f);
}

void SceneBattle::spawnItem(std::shared_ptr<Entity> target, ItemSpec item) {
	auto e = entityManager.addEntity("ITEM", item.id);
	if (item.effect.type == "RESTORE") {
		e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(item.effect.id), true);
	}
	e->addComponent<CLifespan>(25);
	e->getComponent<CLifespan>().remaining = 25;
	e->addComponent<CTransform>(Vec2(target->getComponent<CTransform>().battlePos.x, target->getComponent<CTransform>().battlePos.y), Vec2(0.0f, 0.0f), 0.0f);
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
		enemy->getComponent<CTransform>().battlePos = Vec2(xPos, yPos);
		scale = 1.5f;
	}
	else if (entity == player) {
		xPos = gameEngine->getWidth() * 0.5f;
		yPos = gameEngine->getHeight() * 0.85f;
		player->getComponent<CTransform>().battlePos = Vec2(xPos, yPos);
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

void SceneBattle::queueMessage(const std::string& message, BattleState nextState) {
	nextMessage = message;
	nextBattleState = nextState;
	waitTimer = battlespeed;
	battleState = BattleState::MESSAGE;
}

void SceneBattle::applyMagic(std::shared_ptr<Entity> attacker, std::shared_ptr<Entity> defender, MagicSpec spell) {
	if (attacker->getComponent<CHealth>().currentMana < spell.manacost) {
		battleMessage = "Not enough MP!";
		return;
	}
	else {
		float multiplier = 1.0f;
		attacker->getComponent<CHealth>().currentMana -= spell.manacost;

		// Damaging spells

		if (spell.effect.type == "DAMAGE") {
			spawnSpell(defender, spell);
			if (defender->hasComponent<CWeaknesses>()) {
				std::vector<std::string>& weaknesses = defender->getComponent<CWeaknesses>().weaknesses;
				if (std::find(weaknesses.begin(), weaknesses.end(), spell.effect.id) != weaknesses.end()) {
					multiplier = 1.25f;
				}
			}
			if (defender->hasComponent<CResistances>()) {
				std::vector<std::string>& resistances = defender->getComponent<CResistances>().resistances;
				if (std::find(resistances.begin(), resistances.end(), spell.effect.id) != resistances.end()) {
					multiplier = 0.75f;
				}
			}

			float total = 0.0f;

			if ((spell.effect.magnitude + attacker->getComponent<CStats>().intelligence - defender->getComponent<CStats>().magicdefense) * multiplier <= 0) {
				total = 0.0f;
			}
			else {
				total = (spell.effect.magnitude + attacker->getComponent<CStats>().intelligence - defender->getComponent<CStats>().magicdefense) * multiplier;
			}
			defender->getComponent<CHealth>().current -= total;

			if (spell.effect.id == "FIRE") {
				gameEngine->playSound("FIRE");
				drawDamageNumber(false, static_cast<int>(total), ORANGE);
			}
			else if (spell.effect.id == "ICE") {
				gameEngine->playSound("ICE");
				drawDamageNumber(false, static_cast<int>(total), SKYBLUE);
			}
			else if (spell.effect.id == "LIGHTNING") {
				gameEngine->playSound("LIGHTNING");
				drawDamageNumber(false, static_cast<int>(total), YELLOW);
			}
			else {
				gameEngine->playSound("HIT");
				drawDamageNumber(false, static_cast<int>(total), WHITE);
			}
		}

		// Restorative spells

		if (spell.effect.type == "RESTORE") {
			spawnSpell(attacker, spell);
			CHealth& health = attacker->getComponent<CHealth>();
			health.current += spell.effect.magnitude;
			if (health.current > health.max) {
				health.current = health.max;
			}
			drawDamageNumber(true, -spell.effect.magnitude, GREEN);
			gameEngine->playSound("HEAL");
		}

		// Status effect spells

		if (spell.effect.type == "EFFECT") {
			if (spell.effect.statusType == "DAMAGE") {
				spawnSpell(defender, spell);
				auto& effects = defender->getComponent<CEffects>().effects;

				// Lambda to check to see if there is an existing status effect of the same type, if so then overwrite
				auto it = std::find_if(effects.begin(), effects.end(), [&](const Effect& e) {
					return e.statusType == spell.effect.statusType;
					});
				if (it != effects.end()) {
					*it = spell.effect;
				}
				else {
					defender->getComponent<CEffects>().effects.push_back(spell.effect);
				}
				gameEngine->playSound(spell.effect.id);
			}
			else {
				spawnSpell(attacker, spell);
				auto& effects = attacker->getComponent<CEffects>().effects;

				// Lambda to check to see if there is an existing status effect of the same type, if so then overwrite
				auto it = std::find_if(effects.begin(), effects.end(), [&](const Effect& e) {
					return e.statusType == spell.effect.statusType;
					});
				if (it != effects.end()) {
					*it = spell.effect;
				}
				else {
					attacker->getComponent<CEffects>().effects.push_back(spell.effect);
				}
				gameEngine->playSound(spell.effect.id);
			}
		}
	}
}

void SceneBattle::applyEffect(std::shared_ptr<Entity> target, Effect effect) {
	if (effect.statusType == "DAMAGE") {
		target->getComponent<CHealth>().current -= effect.magnitude;
		spawnEffect(target, effect.id);

		if (effect.id == "POISON") {
			gameEngine->playSound("POISON");
			drawDamageNumber(false, static_cast<int>(effect.magnitude), PURPLE);
		}
		else if (effect.id == "BURN") {
			gameEngine->playSound("BURN");
			drawDamageNumber(false, static_cast<int>(effect.magnitude), ORANGE);
		}
		else if (effect.id == "BLEED") {
			gameEngine->playSound("BLEED");
			drawDamageNumber(false, static_cast<int>(effect.magnitude), RED);
		}
		else {
			gameEngine->playSound("HIT");
			drawDamageNumber(false, static_cast<int>(effect.magnitude), WHITE);
		}
	}

	if (effect.statusType == "HEAL") {
		target->getComponent<CHealth>().current += effect.magnitude;
		if (target->getComponent<CHealth>().current > target->getComponent<CHealth>().max) {
			target->getComponent<CHealth>().current = target->getComponent<CHealth>().max;
		}	
		spawnEffect(target, effect.id);
		gameEngine->playSound(effect.id);
		drawDamageNumber(true, -static_cast<int>(effect.magnitude), GREEN);
	}
}

void SceneBattle::applyDamage(std::shared_ptr<Entity> attacker, std::shared_ptr<Entity> defender) {
	if (attacker->hasComponent<CWeapons>() && attacker->getComponent<CWeapons>().currentWeapon.id != "") {
		float multiplier = 1.0f;
		WeaponSpec& pWeapon = attacker->getComponent<CWeapons>().currentWeapon;

		if (defender->hasComponent<CWeaknesses>()) {
			std::vector<std::string>& weaknesses = defender->getComponent<CWeaknesses>().weaknesses;
			if (std::find(weaknesses.begin(), weaknesses.end(), pWeapon.effect.id) != weaknesses.end()) {
				multiplier = 1.25f;
			}
		}
		if (defender->hasComponent<CResistances>()) {
			std::vector<std::string>& resistances = defender->getComponent<CResistances>().resistances;
			if (std::find(resistances.begin(), resistances.end(), pWeapon.effect.id) != resistances.end()) {
				multiplier = 0.75f;
			}
		}

		float total = 0.0f;

		if ((pWeapon.damage + attacker->getComponent<CStats>().strength * multiplier - defender->getComponent<CStats>().defense) * multiplier <= 0) {
			total = 0.0f;
		}
		else {
			total = (pWeapon.damage + attacker->getComponent<CStats>().strength - defender->getComponent<CStats>().defense) * multiplier;
		}

		if (total <= 0) {
			gameEngine->playSound("NODAMAGE");
		}
		else if (pWeapon.effect.id == "SLASH") {
			gameEngine->playSound("SLASH");
		}
		else if (pWeapon.effect.id == "PIERCE") {
			gameEngine->playSound("PIERCE");
		}
		else if (pWeapon.effect.id == "SMASH") {
			gameEngine->playSound("SMASH");
		}
		else {
			gameEngine->playSound("HIT");
		}
		defender->getComponent<CHealth>().current -= total;
		spawnEffect(defender, pWeapon.effect.id);
		drawDamageNumber(false, static_cast<int>(total), WHITE);
	}
	else {

		float multiplier = 1.0f;
		std::string type = attacker->getComponent<CStats>().baseDamageType;
		std::vector<std::string>& weaknesses = defender->getComponent<CWeaknesses>().weaknesses;
		std::vector<std::string>& resistances = defender->getComponent<CResistances>().resistances;

		if (std::find(weaknesses.begin(), weaknesses.end(), type) != weaknesses.end()) {
			multiplier = 1.25f;
		}
		else if (std::find(resistances.begin(), resistances.end(), type) != resistances.end()) {
			multiplier = 0.75f;
		}

		float total = 0.0f;

		if ((attacker->getComponent<CStats>().strength - defender->getComponent<CStats>().defense) * multiplier <= 0) {
			total = 0.0f;
		}
		else {
			total = (attacker->getComponent<CStats>().strength - defender->getComponent<CStats>().defense) * multiplier;
		}	

		if (total <= 0) {
			gameEngine->playSound("NODAMAGE");
		}
		else if (type == "SLASH") {
			gameEngine->playSound("SLASH");
		}
		else if (type == "PIERCE") {
			gameEngine->playSound("PIERCE");
		}
		else if (type == "SMASH") {
			gameEngine->playSound("SMASH");
		}
		else {
			gameEngine->playSound("HIT");
		}

		spawnEffect(defender, type);

		defender->getComponent<CHealth>().current -= total;
		drawDamageNumber(true, static_cast<int>(total), RED);
	}
}

void SceneBattle::drawDamageNumber(bool playerHit, int damage, Color color) {
	DamageNumber number;

	number.total = 45;
	number.remaining = 45;
	number.velocity = Vec2(0.0f, -1.2f);
	number.color = color;
	
	if (damage < 0) {
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

void SceneBattle::update() {
	entityManager.update();

	sBattle();
	sMovement();
	sLifespan();
	sAnimation();
	sMusic();
	sRender();
}