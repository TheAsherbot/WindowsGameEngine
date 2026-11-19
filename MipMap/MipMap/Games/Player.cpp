#include "Player.h"
#include <cmath>
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace Games
{
	static float Approach(float val, float target, float maxMove)
	{
		return (val > target) ? std::max(val - maxMove, target) : std::min(val + maxMove, target);
	}

	Player::Player()
	{
		scaleX = 1.0f;
		scaleY = 1.0f;
	}

	void Player::Spawn(float x, float y)
	{
		posX = x;
		posY = y;
		vx = 0.0f;
		vy = 0.0f;
		remX = 0.0f;
		remY = 0.0f;
		state = PlayerState::Normal;
		onGround = false;
		onWall = false;
		wallDir = 0;
		lastWallDir = 0;
		dashesAvailable = maxDashes;
		jumpGraceTimer = 0.0f;
		jumpBufferTimer = 0.0f;
		varJumpTimer = 0.0f;
		varJumpSpeed = 0.0f;
		autoJumpTimer = 0.0f;
		wallGraceTimer = 0.0f;
		wallJumpForceTimer = 0.0f;
		forceMoveX = 0;
		wallSlideTimer = WallSlideTime;
		dashTimer = 0.0f;
		dashCooldownTimer = 0.0f;
		dashBufferTimer = 0.0f;
		dashFreezeTimer = 0.0f;
		dashPending = false;
		waveDashTimer = 0.0f;
		waveDashDir = 0;
		waveDashBoostTimer = 0.0f;
		respawnTimer = 0.0f;
		scaleX = 1.0f;
		scaleY = 1.0f;
		screenShake = 0.0f;
		diedThisFrame = false;
		respawnedThisFrame = false;
	}

	void Player::RefillDash()
	{
		dashesAvailable = maxDashes;
		dashCooldownTimer = 0.0f; // Instant readiness to dash again!
	}

	void Player::Kill(ParticleSystem& particles)
	{
		if (state == PlayerState::Dead) return;

		state = PlayerState::Dead;
		diedThisFrame = true;
		respawnTimer = 0.35f; // Fast, snappy Celeste-style respawn
		screenShake = 12.0f;
		particles.SpawnDeathBurst(posX + width / 2.0f, posY + height / 2.0f);
	}

	bool Player::CheckJumpPressed(Input::InputManager* input)
	{
		// ONLY Space, C, Z for jump!
		// W and UpArrow are strictly reserved for aiming up!
		if (input->GetRegisteredKeyState(Input::Space) == Input::Pressed) return true;
		if (input->GetRegisteredKeyState(Input::C) == Input::Pressed) return true;
		if (input->GetRegisteredKeyState(Input::Z) == Input::Pressed) return true;
		return false;
	}

	bool Player::CheckJumpHeld(Input::InputManager* input)
	{
		auto check = [&](Input::Key key) {
			Input::KeyState s = input->GetRegisteredKeyState(key);
			return (s == Input::Pressed || s == Input::Held);
		};
		return check(Input::Space) || check(Input::C) || check(Input::Z);
	}

	bool Player::CheckDashPressed(Input::InputManager* input)
	{
		// J, X, K, Shift, LShift, Left Mouse Click
		if (input->GetRegisteredKeyState(Input::J) == Input::Pressed) return true;
		if (input->GetRegisteredKeyState(Input::X) == Input::Pressed) return true;
		if (input->GetRegisteredKeyState(Input::K) == Input::Pressed) return true;
		if (input->GetRegisteredKeyState(Input::Shift) == Input::Pressed) return true;
		if (input->GetRegisteredKeyState(Input::LShift) == Input::Pressed) return true;
		if (input->GetMouseButtonState(Input::Left) == Input::Pressed) return true;
		return false;
	}

	int Player::GetMoveX(Input::InputManager* input)
	{
		auto held = [&](Input::Key key) {
			Input::KeyState s = input->GetRegisteredKeyState(key);
			return (s == Input::Pressed || s == Input::Held);
		};

		int dir = 0;
		if (held(Input::D) || held(Input::RightArrow)) dir += 1;
		if (held(Input::A) || held(Input::LeftArrow)) dir -= 1;
		return dir;
	}

	int Player::GetMoveY(Input::InputManager* input)
	{
		auto held = [&](Input::Key key) {
			Input::KeyState s = input->GetRegisteredKeyState(key);
			return (s == Input::Pressed || s == Input::Held);
		};

		int dir = 0;
		if (held(Input::S) || held(Input::DownArrow)) dir += 1;
		if (held(Input::W) || held(Input::UpArrow)) dir -= 1;
		return dir;
	}

	void Player::TriggerDash(int inputMoveX, int inputMoveY, ParticleSystem& particles)
	{
		float dx = static_cast<float>(inputMoveX);
		float dy = static_cast<float>(inputMoveY);

		// If no direction held, dash in facing direction
		if (dx == 0.0f && dy == 0.0f)
		{
			dx = static_cast<float>(facingDir);
			dy = 0.0f;
		}
		else
		{
			// Normalize for diagonal dashes
			float length = std::sqrt(dx * dx + dy * dy);
			dx /= length;
			dy /= length;
		}

		dashDirX = dx;
		dashDirY = dy;
		vx = 0.0f;
		vy = 0.0f;
		dashCooldownTimer = DashCooldown;
		dashFreezeTimer = DashFreezeTime; // Celeste hit-stop freeze (0.05s)
		dashPending = true;
		dashBufferTimer = 0.0f; // Consume buffer
		dashesAvailable--;
		screenShake = 5.0f;

		// If dashing along ground or down-diagonal on ground, prime wavedash window
		if (onGround && dy >= 0.0f && dx != 0.0f)
		{
			waveDashTimer = WaveDashWindow;
			waveDashDir = (dx > 0.0f) ? 1 : -1;
		}
	}

	void Player::ExecuteWaveDash(ParticleSystem& particles)
	{
		int dir = (waveDashDir != 0) ? waveDashDir : facingDir;
		vx = dir * WaveDashSpeed; // 1120.0f px/s supersonic boost!
		vy = WaveDashJumpSpeed;   // -360.0f px/s crisp gliding jump arc
		varJumpSpeed = WaveDashJumpSpeed;
		varJumpTimer = WaveDashVarTime;
		autoJumpTimer = 0.0f;
		jumpBufferTimer = 0.0f;
		jumpGraceTimer = 0.0f;
		waveDashTimer = 0.0f;
		waveDashBoostTimer = 0.55f; // Generous window where extreme forward momentum is preserved!
		facingDir = dir;
		state = PlayerState::Normal;
		dashTimer = 0.0f;

		// Refill dash so player has it fully available mid-air!
		RefillDash();

		scaleX = 1.55f;
		scaleY = 0.55f;
		screenShake = 5.0f;

		// Burst particles, dust, and initial ghost trail
		particles.SpawnDust(posX + width / 2.0f, posY + height, static_cast<float>(-dir), -0.4f, 16, Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f));
		particles.SpawnJumpDust(posX, posY + height, width);
		particles.AddGhost(posX, posY, width, height, Renderer::Color(0.2f, 0.9f, 1.0f, 0.85f));
	}

	void Player::Update(float dt, Input::InputManager* input, Level& level, ParticleSystem& particles)
	{
		// Screen shake decay
		if (screenShake > 0.0f)
		{
			screenShake -= dt * 25.0f;
			if (screenShake < 0.0f) screenShake = 0.0f;
		}

		// Death state handling
		if (state == PlayerState::Dead)
		{
			respawnTimer -= dt;
			if (respawnTimer <= 0.0f)
			{
				Spawn(level.GetSpawnX(), level.GetSpawnY());
				level.Reset(); // Instantly reset all dash refill crystals and springs
				respawnedThisFrame = true;
			}
			return;
		}

		// Victory state
		if (state == PlayerState::Victory)
		{
			return;
		}

		// -------------------------------------------------------------
		// READ INPUTS & UPDATE BUFFERS FIRST (Never drop inputs!)
		// -------------------------------------------------------------
		int inputMoveX = GetMoveX(input);
		int inputMoveY = GetMoveY(input);
		bool jumpPressed = CheckJumpPressed(input);
		bool jumpHeld = CheckJumpHeld(input);
		bool dashPressed = CheckDashPressed(input);

		if (inputMoveX != 0)
		{
			facingDir = inputMoveX;
		}

		// Buffer jump & dash inputs
		if (jumpPressed)
		{
			jumpBufferTimer = JumpBufferTime;
		}
		if (dashPressed)
		{
			dashBufferTimer = DashBufferTime;
		}

		// Update timers
		if (jumpBufferTimer > 0.0f) jumpBufferTimer -= dt;
		if (jumpGraceTimer > 0.0f) jumpGraceTimer -= dt;
		if (wallGraceTimer > 0.0f) wallGraceTimer -= dt;
		if (wallJumpForceTimer > 0.0f) wallJumpForceTimer -= dt;
		if (dashBufferTimer > 0.0f) dashBufferTimer -= dt;
		if (dashCooldownTimer > 0.0f) dashCooldownTimer -= dt;
		if (waveDashTimer > 0.0f) waveDashTimer -= dt;
		if (waveDashBoostTimer > 0.0f)
		{
			waveDashBoostTimer -= dt;
			ghostSpawnTimer += dt;
			if (ghostSpawnTimer >= 0.04f)
			{
				ghostSpawnTimer = 0.0f;
				particles.AddGhost(posX, posY, width, height, Renderer::Color(0.2f, 0.9f, 1.0f, 0.70f));
			}
		}

		// Dash freeze-frame hit stop:
		// In Celeste, during freeze frames the player can still change their dash aim!
		if (dashFreezeTimer > 0.0f)
		{
			dashFreezeTimer -= dt;

			// Allow changing dash aim during freeze frames (Celeste lastAim mechanic)
			if (dashPending && (inputMoveX != 0 || inputMoveY != 0))
			{
				float dx = static_cast<float>(inputMoveX);
				float dy = static_cast<float>(inputMoveY);
				float len = std::sqrt(dx * dx + dy * dy);
				dashDirX = dx / len;
				dashDirY = dy / len;
				if (inputMoveX != 0)
				{
					facingDir = inputMoveX;
				}
			}

			if (dashFreezeTimer <= 0.0f && dashPending)
			{
				dashPending = false;
				state = PlayerState::Dashing;
				dashTimer = DashTime;
				vx = dashDirX * DashSpeed;
				vy = dashDirY * DashSpeed;

				// Stretch along final dash direction
				if (std::abs(dashDirX) > 0.0f && dashDirY == 0.0f)
				{
					scaleX = 1.45f; scaleY = 0.65f;
				}
				else if (dashDirX == 0.0f && std::abs(dashDirY) > 0.0f)
				{
					scaleX = 0.65f; scaleY = 1.45f;
				}
				else
				{
					scaleX = 1.25f; scaleY = 1.25f;
				}

				particles.AddGhost(posX, posY, width, height, Renderer::Color(0.0f, 0.75f, 1.0f, 0.8f));
				particles.SpawnDust(posX + width / 2.0f, posY + height / 2.0f, -dashDirX, -dashDirY, 12, Renderer::Color(0.2f, 0.9f, 1.0f, 0.9f));
			}
			return;
		}

		// -------------------------------------------------------------
		// DASH TRIGGER (8-Directional with Input Buffer)
		// -------------------------------------------------------------
		if (dashBufferTimer > 0.0f && dashesAvailable > 0 && dashCooldownTimer <= 0.0f)
		{
			TriggerDash(inputMoveX, inputMoveY, particles);
		}

		// -------------------------------------------------------------
		// DASHING STATE PHYSICS
		// -------------------------------------------------------------
		if (state == PlayerState::Dashing)
		{
			dashTimer -= dt;
			ghostSpawnTimer += dt;
			if (ghostSpawnTimer >= 0.03f)
			{
				ghostSpawnTimer = 0.0f;
				particles.AddGhost(posX, posY, width, height, Renderer::Color(0.0f, 0.75f, 1.0f, 0.8f));
			}

			// -------------------------------------------------------------
			// WALL BOUNCE (SuperWallJump)
			// In Celeste, pressing Jump while dashing upwards alongside a wall
			// triggers a Wall Bounce, giving a massive vertical launch!
			// -------------------------------------------------------------
			if (jumpBufferTimer > 0.0f && dashDirY < 0.0f)
			{
				AABB leftCheck = { posX - 12.0f, posY + 2.0f, 12.0f, height - 4.0f };
				AABB rightCheck = { posX + width, posY + 2.0f, 12.0f, height - 4.0f };
				bool nearLeft = level.CheckSolidOverlap(leftCheck);
				bool nearRight = level.CheckSolidOverlap(rightCheck);

				if (nearLeft || nearRight)
				{
					int bounceWallDir = nearLeft ? -1 : 1;
					int dir = -bounceWallDir; // Away from wall

					state = PlayerState::Normal;
					dashTimer = 0.0f;
					jumpBufferTimer = 0.0f;
					jumpGraceTimer = 0.0f;
					wallGraceTimer = 0.0f;

					vx = dir * SuperWallJumpH;
					vy = SuperWallJumpSpeed; // -640 px/s!
					varJumpSpeed = SuperWallJumpSpeed;
					varJumpTimer = SuperWallJumpVarTime; // 0.25s
					autoJumpTimer = 0.0f;
					forceMoveX = dir;
					wallJumpForceTimer = SuperWallJumpForceTime; // 0.20s

					facingDir = dir;
					scaleX = 0.60f;
					scaleY = 1.45f;
					screenShake = 4.0f;

					particles.SpawnDust(bounceWallDir > 0 ? (posX + width) : posX, posY + height / 2.0f, static_cast<float>(dir), -0.8f, 12, Renderer::Color(1.0f, 0.95f, 0.4f, 1.0f));
					particles.SpawnJumpDust(posX, posY + height, width);
					return;
				}
			}

			// Super / Hyper Dash (Jump during ground dash or down-diagonal dash)
			if (jumpBufferTimer > 0.0f && (onGround || jumpGraceTimer > 0.0f) && (dashDirX != 0.0f && dashDirY >= 0.0f))
			{
				waveDashDir = (dashDirX > 0.0f) ? 1 : -1;
				ExecuteWaveDash(particles);
				return;
			}

			// Constant dash speed, no gravity
			MoveH(vx * dt, level);
			MoveV(vy * dt, level);

			// Ground contact during downward or down-diagonal dash (Celeste Wavedash setup)
			if (dashDirY > 0.0f)
			{
				AABB gBox = { posX + 2.0f, posY + height, width - 4.0f, 2.0f };
				if (level.CheckSolidOverlap(gBox))
				{
					onGround = true;
					jumpGraceTimer = JumpGraceTime;
					RefillDash();

					if (std::abs(dashDirX) > 0.0f)
					{
						waveDashTimer = WaveDashWindow;
						waveDashDir = (dashDirX > 0.0f) ? 1 : -1;
					}

					// If jump is already buffered, immediately launch into WaveDash!
					if (jumpBufferTimer > 0.0f && waveDashTimer > 0.0f)
					{
						ExecuteWaveDash(particles);
						return;
					}

					// Otherwise transition to grounded slide with primed wavedash window
					state = PlayerState::Normal;
					dashTimer = 0.0f;
					vx = dashDirX * 720.0f; // Rapid ground slide while primed for wavedash
					vy = 0.0f;
					scaleX = 1.35f;
					scaleY = 0.70f;
					particles.SpawnDust(posX + width / 2.0f, posY + height, 0.0f, -0.4f, 6);
				}
			}

			if (dashTimer <= 0.0f && state == PlayerState::Dashing)
			{
				// Transition out of dash state
				state = PlayerState::Normal;
				vx = dashDirX * EndDashSpeed;
				if (dashDirY < 0.0f)
				{
					vy = dashDirY * EndDashSpeed * EndDashUpMult;
				}
				else if (dashDirY > 0.0f)
				{
					vy = 0.0f;
				}
			}
		}
		else // --------------------------------------------------------
		     // NORMAL STATE PHYSICS (Celeste Run, Jump, Wall Slide, Wall Jump)
		     // --------------------------------------------------------
		{
			// Determine effective horizontal input direction
			int effectiveMoveX = inputMoveX;
			if (wallJumpForceTimer > 0.0f)
			{
				effectiveMoveX = forceMoveX; // Celeste forceMoveX away from wall
			}

			// Running and Friction (exact Celeste approach logic with momentum preservation)
			float mult = onGround ? 1.0f : AirMult;
			float max = MaxRun;

			if (std::abs(vx) > max)
			{
				int currentSign = (vx > 0.0f) ? 1 : -1;
				if (effectiveMoveX == -currentSign)
				{
					// Actively holding opposite direction: brake strongly to stop / turn around
					vx = Approach(vx, 0.0f, RunAccel * mult * dt);
				}
				else if (effectiveMoveX == currentSign)
				{
					// Holding forward: preserve high momentum!
					if (waveDashBoostTimer > 0.0f)
					{
						// Active supersonic coasting: minimal decay so you fly forward!
						vx = Approach(vx, max * currentSign, RunReduce * 0.15f * dt);
					}
					else
					{
						float decayRate = onGround ? RunReduce : (RunReduce * 0.35f);
						vx = Approach(vx, max * currentSign, decayRate * dt);
					}
				}
				else // effectiveMoveX == 0 (released directional keys)
				{
					if (waveDashBoostTimer > 0.0f)
					{
						// Supersonic glide continues smoothly even if input is released
						vx = Approach(vx, max * currentSign, RunReduce * 0.20f * dt);
					}
					else if (!onGround)
					{
						// In air without input: gentle air gliding (Celeste momentum carry)
						vx = Approach(vx, 0.0f, RunReduce * 0.35f * dt);
					}
					else
					{
						// On ground without input: standard deceleration to standstill
						vx = Approach(vx, 0.0f, RunReduce * dt);
					}
				}
			}
			else
			{
				// Approach target running speed with acceleration
				vx = Approach(vx, max * effectiveMoveX, RunAccel * mult * dt);
			}

			// ---------------------------------------------------------
			// WALL SLIDING: ONLY SLIDE IF PUSHING TOWARDS THE WALL!
			// Slides faster dynamically over time, exactly like Celeste!
			// ---------------------------------------------------------
			bool isSlidingWall = false;
			if (onWall && !onGround && vy > 0.0f)
			{
				// Must explicitly push into the wall (inputMoveX == wallDir)
				if (inputMoveX == wallDir)
				{
					isSlidingWall = true;
					wallSlideTimer -= dt;
					float slideT = std::clamp(wallSlideTimer / WallSlideTime, 0.0f, 1.0f);
					float targetSlideSpeed = WallSlideFastMax + (WallSlideStartMax - WallSlideFastMax) * slideT;
					vy = Approach(vy, targetSlideSpeed, Gravity * 0.7f * dt);

					// Wall slide dust sparks
					particles.SpawnWallSlideDust(wallDir > 0 ? (posX + width) : posX, posY + height * 0.7f, wallDir > 0);
				}
				else
				{
					wallSlideTimer = WallSlideTime;
				}
			}
			else
			{
				wallSlideTimer = WallSlideTime;
			}

			// Gravity application
			if (!isSlidingWall)
			{
				float currentMaxFall = (inputMoveY > 0) ? FastMaxFall : MaxFall;
				float currentGrav = Gravity;

				// Jump Apex Hang (half gravity near jump peak when holding jump)
				if (jumpHeld && std::abs(vy) < HalfGravThreshold)
				{
					currentGrav *= 0.5f;
				}

				if (inputMoveY > 0 && vy > MaxFall)
				{
					vy = std::min(currentMaxFall, vy + FastMaxAccel * dt);
				}
				else
				{
					vy = std::min(currentMaxFall, vy + currentGrav * dt);
				}
			}

			// Variable Jump Height (holding jump or auto-jump maintains initial upward boost)
			if (varJumpTimer > 0.0f)
			{
				if (autoJumpTimer > 0.0f)
				{
					autoJumpTimer -= dt;
					vy = varJumpSpeed;
					varJumpTimer -= dt;
				}
				else if (jumpHeld)
				{
					vy = varJumpSpeed; // Maintain initial jump/bounce speed (Celeste varJumpSpeed)
					varJumpTimer -= dt;
				}
				else
				{
					varJumpTimer = 0.0f; // Cut jump short
				}
			}

			// ---------------------------------------------------------
			// 1. WAVEDASH / HYPER JUMP (with Ground & Coyote Time)
			// Triggered when jumping while primed from a down-diagonal or ground dash!
			// Launches forward at extreme velocity and guarantees dash is ready mid-air!
			// ---------------------------------------------------------
			if (jumpBufferTimer > 0.0f && (onGround || jumpGraceTimer > 0.0f) && waveDashTimer > 0.0f)
			{
				ExecuteWaveDash(particles);
			}
			// ---------------------------------------------------------
			// 2. STANDARD GROUND JUMP (with Coyote Time & Buffer)
			// ---------------------------------------------------------
			else if (jumpBufferTimer > 0.0f && jumpGraceTimer > 0.0f)
			{
				vy = JumpSpeed;
				varJumpSpeed = JumpSpeed;
				varJumpTimer = VarJumpTime;
				autoJumpTimer = 0.0f;
				jumpBufferTimer = 0.0f;
				jumpGraceTimer = 0.0f;
				scaleX = 0.70f;
				scaleY = 1.35f;
				particles.SpawnJumpDust(posX, posY + height, width);
			}
			// ---------------------------------------------------------
			// WALL JUMP (with Wall Coyote Time & Buffer)
			// Pushes significantly further away when holding away from wall!
			// ---------------------------------------------------------
			else if (jumpBufferTimer > 0.0f && (onWall || wallGraceTimer > 0.0f) && !onGround)
			{
				int jumpWallDir = onWall ? wallDir : lastWallDir;
				if (jumpWallDir == 0) jumpWallDir = -facingDir;

				int dir = -jumpWallDir; // Direction away from the wall

				if (inputMoveX == 0)
				{
					// -----------------------------------------------------
					// NEUTRAL WALL JUMP (inputMoveX == 0)
					// Celeste Neutral Jump Mechanic:
					// Releasing directional input before jumping gives a gentle kick off
					// with ZERO forced move timer (wallJumpForceTimer = 0).
					// This allows the player to immediately steer back into the wall
					// at the apex of the jump, gaining height on every neutral wall jump!
					// -----------------------------------------------------
					vx = dir * 280.0f; // Gentle outward kick
					vy = WallJumpSpeed; // Full vertical jump speed (-420.0f)
					varJumpSpeed = WallJumpSpeed;
					varJumpTimer = VarJumpTime;
					autoJumpTimer = 0.0f;
					forceMoveX = 0;
					wallJumpForceTimer = 0.0f; // No forced direction lock!
				}
				else if (inputMoveX == dir)
				{
					// -----------------------------------------------------
					// HOLDING AWAY FROM WALL (inputMoveX == dir)
					// Full launch away from wall to clear large horizontal gaps.
					// -----------------------------------------------------
					vx = dir * (WallJumpHSpeed + WallJumpAwayBonus); // 640.0f
					vy = WallJumpSpeed;
					varJumpSpeed = WallJumpSpeed;
					varJumpTimer = VarJumpTime;
					autoJumpTimer = 0.0f;
					forceMoveX = dir;
					wallJumpForceTimer = WallJumpForceTime; // 0.18s
				}
				else
				{
					// -----------------------------------------------------
					// HOLDING INTO WALL (inputMoveX == -dir)
					// User Requirement: holding into a wall and jumping repeatedly
					// must NOT gain any height!
					// Strong outward launch with full force lock (0.18s) and reduced
					// vertical impulse pushes the player far outward. By the time they
					// steer back to the wall, gravity has pulled them lower than where they jumped.
					// -----------------------------------------------------
					vx = dir * WallJumpHSpeed; // 520.0f outward push
					vy = WallJumpSpeed * 0.70f; // Reduced upward impulse (-294.0f)
					varJumpSpeed = WallJumpSpeed * 0.70f;
					varJumpTimer = VarJumpTime * 0.5f;
					autoJumpTimer = 0.0f;
					forceMoveX = dir;
					wallJumpForceTimer = WallJumpForceTime; // 0.18s locked moving away
				}
				jumpBufferTimer = 0.0f;
				wallGraceTimer = 0.0f;
				waveDashBoostTimer = 0.0f;
				facingDir = dir;

				scaleX = 0.75f;
				scaleY = 1.30f;
				particles.SpawnDust(jumpWallDir > 0 ? (posX + width) : posX, posY + height / 2.0f, static_cast<float>(dir), -0.5f, 8);
			}

			// Perform subpixel swept movement
			float prevBottom = posY + height;
			MoveH(vx * dt, level);
			MoveV(vy * dt, level);

			// Semi-solid platform check
			AABB currentBox = GetBounds();
			if (level.CheckPlatformOverlap(currentBox, prevBottom))
			{
				int platGridY = (static_cast<int>(currentBox.Bottom()) - Level::TileOffsetY) / Level::TileSize;
				posY = static_cast<float>(Level::TileOffsetY + platGridY * Level::TileSize) - height;
				vy = 0.0f;
				onGround = true;
			}
		}

		// -------------------------------------------------------------
		// GROUND & WALL STATE DETECTION
		// -------------------------------------------------------------
		bool wasOnGround = onGround;
		AABB groundCheck = { posX + 2.0f, posY + height, width - 4.0f, 2.0f };
		onGround = level.CheckSolidOverlap(groundCheck);

		if (onGround)
		{
			jumpGraceTimer = JumpGraceTime;
			RefillDash(); // Instantly refill dash on ground!

			// If recently dashed horizontally or down-diagonally, prime the WaveDash!
			if (dashDirX != 0.0f && dashDirY >= 0.0f && dashCooldownTimer > 0.0f)
			{
				waveDashTimer = WaveDashWindow;
				waveDashDir = (dashDirX > 0.0f) ? 1 : -1;
			}

			// If jump is buffered while wavedash is primed, execute immediately!
			if (jumpBufferTimer > 0.0f && waveDashTimer > 0.0f)
			{
				ExecuteWaveDash(particles);
			}

			// If dash was buffered right before landing, fire immediately!
			if (dashBufferTimer > 0.0f)
			{
				TriggerDash(inputMoveX, inputMoveY, particles);
			}

			// Squash upon landing
			if (!wasOnGround && vy >= 0.0f)
			{
				waveDashBoostTimer = 0.0f;
				scaleX = 1.35f;
				scaleY = 0.70f;
				particles.SpawnDust(posX + width / 2.0f, posY + height, 0.0f, -0.4f, 6);
			}
		}

		// Wall checks (10px check distance for generous wall jumps)
		if (!onGround)
		{
			AABB leftCheck = { posX - 10.0f, posY + 2.0f, 10.0f, height - 4.0f };
			AABB rightCheck = { posX + width, posY + 2.0f, 10.0f, height - 4.0f };

			bool leftWall = level.CheckSolidOverlap(leftCheck);
			bool rightWall = level.CheckSolidOverlap(rightCheck);

			if (leftWall)
			{
				onWall = true;
				wallDir = -1;
				lastWallDir = -1;
				wallGraceTimer = WallGraceTime;
			}
			else if (rightWall)
			{
				onWall = true;
				wallDir = 1;
				lastWallDir = 1;
				wallGraceTimer = WallGraceTime;
			}
			else
			{
				onWall = false;
				wallDir = 0;
			}
		}
		else
		{
			onWall = false;
			wallDir = 0;
		}

		// -------------------------------------------------------------
		// INTERACTIVE LEVEL ENTITIES
		// -------------------------------------------------------------
		AABB box = GetBounds();

		// Dash Refill Crystals
		if (level.CheckDashRefillOverlap(box, particles))
		{
			RefillDash(); // Instantly clears cooldown and refills dash!
			scaleX = 1.25f;
			scaleY = 1.25f;

			// If dash is buffered, chain immediately into next dash!
			if (dashBufferTimer > 0.0f)
			{
				TriggerDash(inputMoveX, inputMoveY, particles);
			}
		}

		// Spring Pads
		float bounceVy = 0.0f;
		if (level.CheckSpringOverlap(box, bounceVy))
		{
			state = PlayerState::Normal;
			dashTimer = 0.0f;
			dashPending = false;
			dashFreezeTimer = 0.0f;
			RefillDash();

			vy = bounceVy;
			varJumpSpeed = bounceVy;
			varJumpTimer = SpringVarJumpTime;
			autoJumpTimer = SpringAutoJumpTime;

			scaleX = 0.55f;
			scaleY = 1.65f;
			screenShake = 6.0f;
			particles.SpawnDust(posX + width / 2.0f, posY + height, 0.0f, -1.0f, 16, Renderer::Color::Yellow);
			particles.SpawnJumpDust(posX, posY + height, width);
		}

		// Hazards (Spikes & Pits)
		if (level.CheckHazardOverlap(box))
		{
			Kill(particles);
			return;
		}

		// Level Goal Portal
		if (level.CheckGoalOverlap(box))
		{
			state = PlayerState::Victory;
			particles.SpawnCrystalCollect(posX + width / 2.0f, posY + height / 2.0f);
			return;
		}

		// Smooth squash & stretch lerp back to 1.0
		scaleX += (1.0f - scaleX) * 15.0f * dt;
		scaleY += (1.0f - scaleY) * 15.0f * dt;
	}

	void Player::MoveH(float amount, Level& level)
	{
		remX += amount;
		int move = static_cast<int>(std::round(remX));
		if (move == 0) return;

		remX -= static_cast<float>(move);
		int step = (move > 0) ? 1 : -1;

		for (int i = 0; i < std::abs(move); i++)
		{
			AABB nextBox = { posX + step, posY, width, height };

			if (!level.CheckSolidOverlap(nextBox))
			{
				posX += static_cast<float>(step);
			}
			else
			{
				// Horizontal Dash Corner Correction (Celeste ledge snap)
				if (state == PlayerState::Dashing && dashDirY == 0.0f)
				{
					bool corrected = false;
					for (int nudge = 1; nudge <= 16; nudge++)
					{
						// Snap up onto platform
						if (!level.CheckSolidOverlap({ posX + step, posY - nudge, width, height }))
						{
							posY -= static_cast<float>(nudge);
							posX += static_cast<float>(step);
							corrected = true;
							break;
						}
						// Snap down under ceiling
						if (!level.CheckSolidOverlap({ posX + step, posY + nudge, width, height }))
						{
							posY += static_cast<float>(nudge);
							posX += static_cast<float>(step);
							corrected = true;
							break;
						}
					}
					if (corrected) continue;
				}

				vx = 0.0f;
				remX = 0.0f;
				break;
			}
		}
	}

	void Player::MoveV(float amount, Level& level)
	{
		remY += amount;
		int move = static_cast<int>(std::round(remY));
		if (move == 0) return;

		remY -= static_cast<float>(move);
		int step = (move > 0) ? 1 : -1;

		for (int i = 0; i < std::abs(move); i++)
		{
			AABB nextBox = { posX, posY + step, width, height };

			if (!level.CheckSolidOverlap(nextBox))
			{
				posY += static_cast<float>(step);
			}
			else
			{
				// Head Bonk Corner Correction (Celeste ceiling edge slide)
				if (step < 0)
				{
					bool corrected = false;
					for (int nudge = 1; nudge <= 16; nudge++)
					{
						if (!level.CheckSolidOverlap({ posX + nudge, posY + step, width, height }))
						{
							posX += static_cast<float>(nudge);
							posY += static_cast<float>(step);
							corrected = true;
							break;
						}
						if (!level.CheckSolidOverlap({ posX - nudge, posY + step, width, height }))
						{
							posX -= static_cast<float>(nudge);
							posY += static_cast<float>(step);
							corrected = true;
							break;
						}
					}
					if (corrected) continue;
				}

				vy = 0.0f;
				remY = 0.0f;
				break;
			}
		}
	}

	void Player::Render(Renderer::DX12Renderer* renderer)
	{
		if (state == PlayerState::Dead) return;

		// Calculate scaled draw size around bottom-center anchor
		float drawW = width * scaleX;
		float drawH = height * scaleY;
		float drawX = posX + (width - drawW) * 0.5f;
		float drawY = posY + (height - drawH);

		int x1 = static_cast<int>(drawX);
		int y1 = static_cast<int>(drawY);
		int x2 = static_cast<int>(drawX + drawW);
		int y2 = static_cast<int>(drawY + drawH);

		// Cube color based on Dash availability
		Renderer::Color cubeColor;
		Renderer::Color borderColor;

		if (dashesAvailable > 0)
		{
			// Electric Cyan when Dash is ready
			cubeColor = Renderer::Color(0.0f, 0.75f, 1.0f, 1.0f);
			borderColor = Renderer::Color(0.6f, 0.95f, 1.0f, 1.0f);
		}
		else
		{
			// Vivid Ruby / Magenta when Dash is spent
			cubeColor = Renderer::Color(0.95f, 0.25f, 0.38f, 1.0f);
			borderColor = Renderer::Color(1.0f, 0.65f, 0.75f, 1.0f);
		}

		// Draw SOLID square with crisp border (NO EYES, NO FACE, CLEAN SOLID GEOMETRY)
		renderer->DrawFilledRectangle(x1, y1, x2, y2, cubeColor);
		renderer->DrawRectangle(x1, y1, x2, y2, borderColor);
		renderer->DrawRectangle(x1 + 1, y1 + 1, x2 - 1, y2 - 1, borderColor);
	}
}
