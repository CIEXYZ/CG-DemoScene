#include <glm/glm.hpp>
#include <vector>
#include <random>

class PoissonDiskSampler {
public:
    struct Rect {
        float x, y, width, height;

        Rect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    };

    // Generate samples with gap
    static std::vector<glm::vec2> generate(const Rect& bounds, float minDistance, int maxAttempts = 30, const Rect* excludeZone = nullptr);

private:
    static bool isValidPoint(
        const glm::vec2& point, const Rect& bounds, float minDistance,
        const std::vector<glm::vec2>& points, const std::vector<std::vector<int>>& grid, float cellSize, const Rect* excludeZone
    );

    static glm::vec2 gridCoords(const glm::vec2& point, const Rect& bounds, float cellSize);
};