#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

using namespace dae;

struct Camera
{
	Camera() = default;

	Camera(const Vector3& _origin, float _fovAngle) :
		origin{ _origin },
		fovAngle{ _fovAngle }
	{
	}


	Vector3 origin{};
	float fovAngle{ 90.f };
	float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
	float aspectRatio;

	Vector3 forward{ Vector3::UnitZ };
	Vector3 up{ Vector3::UnitY };
	Vector3 right{ Vector3::UnitX };

	float totalPitch{};
	float totalYaw{};

	Matrix invViewMatrix{};
	Matrix viewMatrix{};
	Matrix projectionMatrix{};

	float farPlane{ 100.f };
	float nearPlane{ .1f };

	void Initialize(float ar, float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f })
	{
		fovAngle = _fovAngle;
		fov = tanf((fovAngle * TO_RADIANS) / 2.f);
		aspectRatio = ar;
		origin = _origin;
	}

	void CalculateViewMatrix()
	{
		//ONB => invViewMatrix
		//Inverse(ONB) => ViewMatrix

		Matrix rot{ Matrix::CreateRotation(Vector3(totalPitch, totalYaw,0)) };
		forward = rot.TransformVector(Vector3::UnitZ);
		forward.Normalize();

		invViewMatrix = Matrix::CreateLookAtLH(origin, forward, Vector3::UnitY);

		viewMatrix = Matrix::Inverse(invViewMatrix);

		right = viewMatrix.GetAxisX();
		up = viewMatrix.GetAxisY();
		//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
	}

	void CalculateProjectionMatrix()
	{

		projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
		//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
		//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
	}

	void Update(const Timer* pTimer)
	{
		const float deltaTime = pTimer->GetElapsed();

		//Literal values, stopped when movement felt ok. Not sure about a permanent solution
		const float baseMoveSpeed = 0.5f;
		const float baseRotSpeed = 0.5f;;
		const float FPSToConsider = 30.f;
		//Super high framerate "fix", otherwise camera doesn't move
		//
		float moveSpeed{ baseMoveSpeed *std::max(deltaTime, 1.f / FPSToConsider) };
		float rotSpeed{ baseRotSpeed * std::max(deltaTime, 1.f / FPSToConsider) };

		//Keyboard Input
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

		if (pKeyboardState[SDL_SCANCODE_LSHIFT])
		{
			moveSpeed *= 2;
		}
		if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
		{
			origin += forward * moveSpeed;
		}
		if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
		{
			origin -= forward * moveSpeed;
		}
		if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
		{
			origin -= right * moveSpeed ;
		}
		if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
		{
			origin += right * moveSpeed ;
		}



		//Mouse Input
		int mouseX{}, mouseY{};
		const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
		moveSpeed = baseMoveSpeed * std::max(deltaTime, 1.f / FPSToConsider);
		

		if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) && mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			if (mouseY > 0)
			{
				origin -= up * moveSpeed;
			}
			if (mouseY < 0)
			{
				origin += up * moveSpeed;
			}
			if (mouseX > 0)
			{
				origin += right * moveSpeed ;
			}
			if (mouseX < 0)
			{
				origin -= right * moveSpeed;
			}

		}
		else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			if (mouseY > 0)
			{
				origin -= forward * moveSpeed ;
			}
			if (mouseY < 0)
			{
				origin += forward * moveSpeed ;
			}
			if (mouseX < 0)
			{
				totalYaw -= rotSpeed;
			}
			if (mouseX > 0)
			{
				totalYaw += rotSpeed;
			}
		}
		else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			if (mouseX < 0)
			{
				totalYaw -= rotSpeed;

			}
			if (mouseX > 0)
			{
				totalYaw += rotSpeed;

			}
			if (mouseY > 0)
			{
				totalPitch -= rotSpeed;

			}
			if (mouseY < 0)
			{
				totalPitch += rotSpeed;

			}
		}


		//Camera Update Logic
		//...

		//Update Matrices
		CalculateViewMatrix();
		CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
	}
};

	
