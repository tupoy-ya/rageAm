//
// File: vector3.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "DirectXMath.h"

namespace rage
{
	struct Vec3V;

	/**
	 * \remarks Use Vec3V for math operations.
	 */
	struct Vector3
	{
		float X, Y, Z;

		Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }
		Vector3(const Vec3V& vec);
		Vector3() : Vector3(0, 0, 0) {}
	};

	/**
	 * \brief SIMD-Accelerated vector for fast math operations.
	 * It has additional W (unused) coordinate to fit into __m128 and also aligned to 16 bytes (sse requirement).
	 */
	struct alignas(16) Vec3V
	{
		using DXVec = DirectX::XMVECTOR;
		using FDXVec = DirectX::FXMVECTOR;

		float X, Y, Z, W;

		Vec3V(float x, float y, float z) { X = x; Y = y; Z = z; W = x; }
		Vec3V(DXVec vec) { SetDx(vec); }
		Vec3V(Vector3 vec) : Vec3V(vec.X, vec.Y, vec.Z) {}
		Vec3V() : Vec3V(0, 0, 0) {}

		void Normalize() { *this = DirectX::XMVector3Normalize(GetDx()); }
		void NormalizeFast() { *this = DirectX::XMVector3Normalize(GetDx()); }
		Vec3V Normalized() const { return DirectX::XMVector3NormalizeEst(GetDx()); }
		Vec3V NormalizedFast() const { return DirectX::XMVector3NormalizeEst(GetDx()); }

		float LengthSquared() const { return DirectX::XMVector3LengthSq(GetDx()).m128_f32[0]; }
		float LengthFast() const { return DirectX::XMVector3LengthEst(GetDx()).m128_f32[0]; }
		float Length() const { return DirectX::XMVector3Length(GetDx()).m128_f32[0]; }

		float Dot(const Vec3V& other) const { return DirectX::XMVector3Dot(GetDx(), other.GetDx()).m128_f32[0]; }
		Vec3V Cross(const Vec3V& other) const { return DirectX::XMVector3Cross(GetDx(), other.GetDx()); }

		Vec3V Reflect(const Vec3V& normal) const { return DirectX::XMVector3Reflect(GetDx(), normal.GetDx()); }

		Vec3V operator*(float scalar) const { return DirectX::XMVectorMultiply(GetDx(), DirectX::XMVectorReplicate(scalar)); }
		Vec3V operator/(float scalar) const { return DirectX::XMVectorDivide(GetDx(), DirectX::XMVectorReplicate(scalar)); }
		Vec3V& operator*=(float scalar) { SetDx(DirectX::XMVectorMultiply(GetDx(), DirectX::XMVectorReplicate(scalar))); return *this; }
		Vec3V& operator/=(float scalar) { SetDx(DirectX::XMVectorDivide(GetDx(), DirectX::XMVectorReplicate(scalar))); return *this; }

		Vec3V operator+(const Vec3V& other) const { return DirectX::XMVectorAdd(GetDx(), other.GetDx()); }
		Vec3V operator-(const Vec3V& other) const { return DirectX::XMVectorSubtract(GetDx(), other.GetDx()); }
		Vec3V& operator+=(const Vec3V& other) { SetDx(DirectX::XMVectorAdd(GetDx(), other.GetDx())); return *this; }
		Vec3V& operator-=(const Vec3V& other) { SetDx(DirectX::XMVectorSubtract(GetDx(), other.GetDx())); return *this; }

		DXVec GetDx() const { return *reinterpret_cast<const DXVec*>(this); }
		void SetDx(DXVec vec) { *reinterpret_cast<DXVec*>(this) = vec; }
	};
}
