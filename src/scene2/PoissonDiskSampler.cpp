#include "PoissonDiskSampler.h"
#include <cmath>
#include <algorithm>
#include <vector>

std::vector<glm::vec2> PoissonDiskSampler::generate(const Rect& bounds, float minDistance, int maxAttempts, const Rect* excludeZone) {
    std::vector<glm::vec2> points;
    std::vector<glm::vec2> activeList;

    //Init grid and cell size
    float cellSize = minDistance / std::sqrt(2.0f);
    int gridWidth = static_cast<int>(std::ceil(bounds.width / cellSize));
    int gridHeight = static_cast<int>(std::ceil(bounds.height / cellSize));

    std::vector<std::vector<int>> grid(gridWidth, std::vector<int>(gridHeight, -1));

    //rand
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> distX(0.0f, bounds.width);
    std::uniform_real_distribution<float> distY(0.0f, bounds.height);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> distRadius(minDistance, 2.0f * minDistance);

    // Generate initial point
    glm::vec2 initialPoint;

    //Check if initial point not in exclude zone
    while (true) {
        initialPoint = glm::vec2(distX(gen), distY(gen));
        if (!excludeZone ||
            initialPoint.x < excludeZone->x ||
            initialPoint.x > excludeZone->x + excludeZone->width ||
            initialPoint.y < excludeZone->y ||
            initialPoint.y > excludeZone->y + excludeZone->height) {
            break;
        }
    }

    points.push_back(initialPoint);
    activeList.push_back(initialPoint);

    //Init grid
    glm::vec2 gridPos = gridCoords(initialPoint, bounds, cellSize);
    grid[static_cast<int>(gridPos.x)][static_cast<int>(gridPos.y)] = 0;

    // Generate points
    while (!activeList.empty()) {
        int randomIndex = std::uniform_int_distribution<int>(0, activeList.size() - 1)(gen);
        glm::vec2 point = activeList[randomIndex];

        bool found = false;

        for (int i = 0; i < maxAttempts; ++i) {
            float angle = distAngle(gen);
            float radius = distRadius(gen);

            glm::vec2 newPoint = point + glm::vec2(
                radius * std::cos(angle),
                radius * std::sin(angle)
            );

            //Check if found point not in exclude zone
            if (isValidPoint(newPoint, bounds, minDistance, points, grid, cellSize, excludeZone)) {
                points.push_back(newPoint);
                activeList.push_back(newPoint);

                glm::vec2 newGridPos = gridCoords(newPoint, bounds, cellSize);
                grid[static_cast<int>(newGridPos.x)][static_cast<int>(newGridPos.y)] = points.size() - 1;

                found = true;
                break;
            }
        }

        if (!found) {
            activeList.erase(activeList.begin() + randomIndex);
        }
    }

    return points;
}

bool PoissonDiskSampler::isValidPoint(const glm::vec2& point, const Rect& bounds, float minDistance,
    const std::vector<glm::vec2>& points, const std::vector<std::vector<int>>& grid, float cellSize, const Rect* excludeZone)
{
    // Check bounds
    if (point.x < 0 || point.x >= bounds.width ||
        point.y < 0 || point.y >= bounds.height) {
        return false;
    }

    // Check exclude zone
    if (excludeZone) {
        if (point.x >= excludeZone->x &&
            point.x <= excludeZone->x + excludeZone->width &&
            point.y >= excludeZone->y &&
            point.y <= excludeZone->y + excludeZone->height) {
            return false;
        }
    }

    // Check distance to existing points
    glm::vec2 gridPos = gridCoords(point, bounds, cellSize);
    int gridX = static_cast<int>(gridPos.x);
    int gridY = static_cast<int>(gridPos.y);

    int searchRadius = 2;

    for (int x = std::max(0, gridX - searchRadius);
         x <= std::min(static_cast<int>(grid.size()) - 1, gridX + searchRadius);
         ++x) {
        for (int y = std::max(0, gridY - searchRadius);
             y <= std::min(static_cast<int>(grid[0].size()) - 1, gridY + searchRadius);
             ++y) {
            int pointIndex = grid[x][y];
            if (pointIndex != -1) {
                float dist = glm::distance(point, points[pointIndex]);
                if (dist < minDistance) {
                    return false;
                }
            }
        }
    }

    return true;
}

glm::vec2 PoissonDiskSampler::gridCoords(const glm::vec2& point, const Rect& bounds, float cellSize) {
    return glm::vec2(
        std::floor(point.x / cellSize),
        std::floor(point.y / cellSize)
    );
}