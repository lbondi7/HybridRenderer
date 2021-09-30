#pragma once

#include "Constants.h"

struct Transform {

	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);

	glm::vec3 right = glm::vec3(1, 0, 0);
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 forward = glm::vec3(0, 0, 1);


	bool operator == (const Transform& other) {
		return position == other.position && rotation == other.rotation && scale == other.scale;
	}

	bool operator != (const Transform& other) {
		return position != other.position || rotation != other.rotation || scale != other.scale;
	}

	Transform& operator = (const Transform& other) {
		position = other.position;
		rotation = other.rotation;
		scale = other.scale;
		return *this;
	}

	void getMatrix(glm::mat4& matrix) {
		matrix = glm::translate(glm::mat4(1.0f), position);
		//matrix *= glm::yawPitchRoll(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
		matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		matrix = glm::scale(matrix, scale);
		glm::mat4 inverse = glm::inverse(matrix);
		right = inverse[0];
		up = inverse[1];
		forward = inverse[2];
	}

	void getMatrix(glm::mat4& matrix, const glm::mat4& parent) {
		matrix = parent;
		matrix *= glm::translate(glm::mat4(1.0f), position);
		//matrix *= glm::yawPitchRoll(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
		matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		matrix = glm::scale(matrix, scale);
		glm::mat4 inverse = glm::inverse(matrix);
		right = inverse[0];
		up = inverse[1];
		forward = inverse[2];
	}

	const glm::mat4 getMatrix() {
		glm::mat4 matrix = glm::mat4(1.0f);
		matrix = glm::translate(glm::mat4(1.0f), position);
		//matrix *= glm::yawPitchRoll(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
		matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		matrix = glm::scale(matrix, scale);
		return matrix;
	}
};
