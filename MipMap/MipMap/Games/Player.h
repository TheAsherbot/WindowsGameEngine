#pragma once
#include "Engine/Renderer/DirectX/DX12Renderer.h"
#include "Engine/Input/Win32Input.h"
#include "Collider.h"
#include "Level.h"
#include "Particles.h"

namespace Games
{
	enum class PlayerState
	{
		Normal,
		Dashing,
		Dead,
		Victory
	};

	class Player
	{
	public:
		// Exact Celeste Physics Constants (Scaled 4x for 1280x720)
		static constexpr float MaxRun = 360.0f;           // 90 * 4
		static constexpr float RunAccel = 4000.0f;        // 1000 * 4
		static constexpr float RunReduce = 1600.0f;       // 400 * 4
		static constexpr float AirMult = 0.65f;

		static constexpr float Gravity = 3600.0f;         // 900 * 4
		static constexpr float MaxFall = 640.0f;          // 160 * 4
		static constexpr float FastMaxFall = 960.0f;      // 240 * 4
		static constexpr float FastMaxAccel = 1200.0f;    // 300 * 4
		static constexpr float HalfGravThreshold = 160.0f;// 40 * 4 (apex hang)

		static constexpr float JumpSpeed = -420.0f;       // -105 * 4
		static constexpr float VarJumpTime = 0.20f;
		static constexpr float JumpGraceTime = 0.12f;     // Coyote time (generous ~7-8 frames)
		static constexpr float JumpBufferTime = 0.12f;    // Jump buffering

		static constexpr float WallJumpHSpeed = 520.0f;   // (90 + 40) * 4
		static constexpr float WallJumpAwayBonus = 120.0f;// Extra kick when holding away from wall (640 px/s total)
		static constexpr float WallJumpSpeed = -420.0f;   // -105 * 4
		static constexpr float WallJumpForceTime = 0.18f; // Horizontal input force
		static constexpr float WallSlideStartMax = 180.0f;// Initial brisk slide speed (45 * 4)
		static constexpr float WallSlideFastMax = 360.0f; // Fast slide speed (90 * 4)
		static constexpr float WallSlideTime = 1.2f;      // Slide acceleration duration
		static constexpr float WallGraceTime = 0.12f;     // Wall coyote time

		// Wall Bounce (Super Wall Jump - upward dash into wall jump)
		static constexpr float SuperWallJumpSpeed = -640.0f;     // -160 * 4
		static constexpr float SuperWallJumpVarTime = 0.25f;
		static constexpr float SuperWallJumpForceTime = 0.20f;
		static constexpr float SuperWallJumpH = 680.0f;          // (90 + 40*2) * 4

		// Spring & Super Bounce
		static constexpr float SpringBounceSpeed = -1150.0f;     // High soaring bounce to clear tall spike walls
		static constexpr float SpringVarJumpTime = 0.26f;        // Window to hold jump for maximum height
		static constexpr float SpringAutoJumpTime = 0.14f;       // Minimum guaranteed high boost even without holding jump

		// Wavedash & Hyperdash Physics (Celeste Speed: 325 * 4 = 1300, tuned to 1120 for precision)
		static constexpr float WaveDashSpeed = 1120.0f;          // Supersonic horizontal launch (approx 3.1x normal run)
		static constexpr float WaveDashJumpSpeed = -360.0f;      // Crisp, gliding trajectory
		static constexpr float WaveDashWindow = 0.32f;           // Generous ~20-frame window to jump after landing from a down-diagonal dash
		static constexpr float WaveDashVarTime = 0.25f;          // Variable jump hold time

		static constexpr float DashSpeed = 960.0f;        // 240 * 4
		static constexpr float EndDashSpeed = 640.0f;     // 160 * 4
		static constexpr float EndDashUpMult = 0.75f;
		static constexpr float DashTime = 0.15f;
		static constexpr float DashCooldown = 0.20f;
		static constexpr float DashFreezeTime = 0.05f;    // Hit stop freeze frame (Celeste Freeze)
		static constexpr float DashBufferTime = 0.12f;    // Dash buffering

		Player();

		void Spawn(float x, float y);
		void Update(float dt, Input::InputManager* input, Level& level, ParticleSystem& particles);
		void Render(Renderer::DX12Renderer* renderer);

		void RefillDash();
		void Kill(ParticleSystem& particles);
		bool IsDead() const { return state == PlayerState::Dead; }
		bool HasDiedThisFrame() { bool d = diedThisFrame; diedThisFrame = false; return d; }
		bool HasRespawnedThisFrame() { bool r = respawnedThisFrame; respawnedThisFrame = false; return r; }
		bool HasReachedGoal() const { return state == PlayerState::Victory; }

		AABB GetBounds() const { return { posX, posY, width, height }; }
		float GetShake() const { return screenShake; }

		int GetDashesAvailable() const { return dashesAvailable; }
		float GetX() const { return posX; }
		float GetY() const { return posY; }

	private:
		// Position & Size
		float posX = 64.0f;
		float posY = 500.0f;
		float width = 28.0f;
		float height = 28.0f;
		float remX = 0.0f; // Subpixel accumulation
		float remY = 0.0f;

		// Velocity
		float vx = 0.0f;
		float vy = 0.0f;
		int facingDir = 1; // 1 = right, -1 = left

		// States & Flags
		PlayerState state = PlayerState::Normal;
		bool onGround = false;
		bool onWall = false;
		int wallDir = 0; // -1 left wall, +1 right wall
		int dashesAvailable = 1;
		int maxDashes = 1;
		bool diedThisFrame = false;
		bool respawnedThisFrame = false;

		// Timers
		float jumpGraceTimer = 0.0f;      // Coyote time
		float jumpBufferTimer = 0.0f;     // Jump buffer
		float varJumpTimer = 0.0f;        // Variable jump height hold
		float varJumpSpeed = 0.0f;        // Upward velocity maintained during variable jump
		float autoJumpTimer = 0.0f;       // Auto jump duration for springs & bounces
		float wallGraceTimer = 0.0f;      // Wall coyote time
		int lastWallDir = 0;
		float wallJumpForceTimer = 0.0f;  // Force move away from wall
		int forceMoveX = 0;               // Direction of forced move
		float wallSlideTimer = 1.2f;      // Wall slide acceleration timer
		float dashTimer = 0.0f;           // Duration of dash
		float dashCooldownTimer = 0.0f;   // Cooldown between dashes
		float dashBufferTimer = 0.0f;     // Buffered dash input
		float dashFreezeTimer = 0.0f;     // Freeze frame
		bool dashPending = false;         // Dash aiming during freeze frame
		float waveDashTimer = 0.0f;       // Wavedash / hyper jump primed window
		int waveDashDir = 0;              // Wavedash launch direction
		float waveDashBoostTimer = 0.0f;  // Active supersonic coasting duration
		float respawnTimer = 0.0f;        // Death delay before respawn
		float screenShake = 0.0f;
		float ghostSpawnTimer = 0.0f;

		// Dash direction
		float dashDirX = 0.0f;
		float dashDirY = 0.0f;

		// Squash & Stretch Juice
		float scaleX = 1.0f;
		float scaleY = 1.0f;

		// Subpixel Swept Movement
		void MoveH(float amount, Level& level);
		void MoveV(float amount, Level& level);

		// Dash Execution
		void TriggerDash(int moveX, int moveY, ParticleSystem& particles);
		void ExecuteWaveDash(ParticleSystem& particles);

		// Helper Input Checks
		bool CheckJumpPressed(Input::InputManager* input);
		bool CheckJumpHeld(Input::InputManager* input);
		bool CheckDashPressed(Input::InputManager* input);
		int GetMoveX(Input::InputManager* input);
		int GetMoveY(Input::InputManager* input);
	};
}
