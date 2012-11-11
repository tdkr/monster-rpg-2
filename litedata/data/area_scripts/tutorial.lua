music = "tutorial.ogg"

stage = -2

function start()
end

function stop()
end

loops = 0

function update(step)
	if (loops == 1) then
		loops = loops + 1
		local ret
		if (get_use_dpad()) then
			ret = anotherDoDialogue("Welcome to the tutorial...\nThis tutorial will show you how to control your character and navigate the menus in this game.\nLet's learn how to walk...\nUse the arrows to walk over to the grass now!\n")
		else
			ret = anotherDoDialogue("Welcome to the tutorial...\nThis tutorial will show you how to control your character and navigate the menus in this game.\nLet's learn how to walk...\nSimply touch your screen at the point you wish to walk to.\nWalk over to the grass now!\n")
		end
		if (ret) then
			if (prompt("Really exit", "tutorial?", 0, 1)) then
				stage = 5;
				return;
			end
		end
	end
	loops = loops + 1

	if (loops <= 5) then
		return
	end

	local px, py = getObjectPosition(0)

	if (stage == -2) then
		if (not isSpeechDialogActive()) then
			stage = stage + 1
		end
	elseif (stage == -1) then
		if (_shake_dialog == nil) then
			local ret
			if (get_use_dpad()) then
				ret = anotherDoDialogue("Note: you can back out of the tutorial at any time by pressing the cancel button.\n")
			else
				if (get_platform() == "iphone") then
					ret = doShakeDialogue("Note: you can back out of the tutorial at any time by shaking your device...\n(and if you don't like shaking your iPhone, you can press the top left corner instead)\n", true)
				else
					ret = anotherDoDialogue("Note: you can back out of the tutorial at any time by pressing the cancel button.\nIt's the red triangle in the top left corner of the screen.\n")
				end
			end
			if (ret) then
				if (prompt("Really exit", "tutorial?", 0, 1)) then
					stage = 5;
					return;
				end
			end
			_shake_dialog = true
		end
		if (not isSpeechDialogActive()) then
			stage = stage + 1
		end
	elseif (stage == 0) then
		if (px >= 14 and px <= 15 and py >= 9 and py <= 10) then
			stage = stage + 1
			local ret;
			if (get_use_dpad()) then
				ret = anotherDoDialogue("Great!\nYou can look around you by holding the action button and using the arrows. Try it now!\n")
			else
				ret = anotherDoDialogue("Great!\nYou can look around you by touching the screen and dragging it to pan. Try it now!\n")
			end
			if (ret) then
				if (prompt("Really exit", "tutorial?", 0, 1)) then
					stage = 5;
					return;
				end
			end
			pan_count = 0
			panned = false
			pan_start_x, pan_start_y = getAreaPan()
		end
	end

	if (stage == 1) then
		if (not panned) then
			local x, y = getAreaPan()
			local dx = pan_start_x-x
			local dy = pan_start_y-y
			if ((dx*dx) > 25 or (dy*dy) > 25) then
				panned = true
			end
		else
			pan_count = pan_count + step
			if (pan_count >= 10000 and not dontComeHereAgain) then
				dontComeHereAgain = true
				local ret = anotherDoDialogue("The big map may be confusing. Let's see how that works now.\n");
				if (ret) then
					if (prompt("Really exit", "tutorial?", 0, 1)) then
						stage = 5;
						return;
					end
				end
				if (doMapTutorial()) then
					stage = 5
				else
					stage = stage + 1
				end
			end
		end
	elseif (stage == 2 and _blah == nil) then
		_blah = true
		local ret
		if (get_use_dpad()) then
			ret = anotherDoDialogue("Nice!\nNow let's get familiar with some user interface features.\nI'm going to open the item menu now, so follow along.\nIn the game you can use the cancel button to open the pause menu where you can access your inventory.\nI'll do it for you this time.\n")
		else
			if (get_platform() == "iphone") then
				ret = anotherDoDialogue("Nice!\nNow let's get familiar with some user interface features.\nI'm going to open the item menu now, so follow along.\nIn the game you can shake your iPhone or iPod to open the pause menu where you can access your inventory.\nI'll do it for you this time.\n")
			else
				ret = anotherDoDialogue("Nice!\nNow let's get familiar with some user interface features.\nI'm going to open the item menu now, so follow along.\nIn the game you can use the cancel button to open the pause menu where you can access your inventory.\nI'll do it for you this time.\n")
			end
		end
		if (ret) then
			if (prompt("Really exit", "tutorial?", 0, 1)) then
				stage = 5;
				return;
			end
		end
		if (doItemTutorial()) then
			stage = 5
		else
			stage = stage + 1
		end
	elseif (stage == 3) then
		stage = stage + 1
		setObjectPosition(0, 15, 10)
		setInventory(0, ITEM_CURE, 1)
		clearBuffer(0, 0, 0)
		drawArea()
		flip()
		startBattle("3Leechs", false)
	elseif (stage == 4) then
		if (not inBattle()) then
			stage = stage + 1
			local ret
			clearBuffer(0, 0, 0)
			drawArea()
			flip()
			if (battleWon()) then
				ret = anotherDoDialogue("Congratulations! Now you should know enough to get started!\n")
			else
				ret = anotherDoDialogue("Congratulations! You didn't win, but you should know enough to get started!\n")
			end
			if (ret) then
				if (prompt("Really exit", "tutorial?", 0, 1)) then
					stage = 5;
					return;
				end
			end
			dpad_off()
			drawArea()
			dpad_on()
			fadeOut(0, 0, 0)
		end
	end
end

function activate(activator, activated)
end

function collide(id1, id2)
end
