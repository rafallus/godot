#ifndef NAVGRID_H
#define NAVGRID_H

#include "scene/main/node.h"
#include "scene/resources/3d/mesh_library.h"

class NavGrid : public Node {
	GDCLASS(NavGrid, Node);

private:
    friend class GridMap;

    const int MAX_GRID_COORD = 128; // -128 to 127

    enum {
        CELL_EMPTY,     // Empty cell.
        CELL_OCCUPIED,  // An agent is idle occupying the cell.
        CELL_OWNED,     // An agent is passing by.
        CELL_BLOCKED,   // An agent is passing by, but is waiting for its path to be cleared.
        CELL_HIDDEN,    // Cell is hidden i.e. high-res pathfinding has been disabled.
        CELL_REMOVED    // Cell has been removed from the grid.
    };

    union GridKey {
        int16_t id = 0;
        struct {
            int8_t x;
            int8_t y;
        };

        static uint32_t hash(const GridKey &p_key) {
            return p_key.id;
        };
        _FORCE_INLINE_ bool operator<(const GridKey &p_key) const {
			return id < p_key.id;
		}
		_FORCE_INLINE_ bool operator==(const GridKey &p_key) const {
			return id == p_key.id;
		}
        _FORCE_INLINE_ operator Vector2i() const {
            return Vector2i(x, y);
        }

        GridKey(Vector2i p_vector) {
            x = (int8_t)p_vector.x;
            y = (int8_t)p_vector.y;
        }
        GridKey(int8_t p_x, int8_t p_y) {
            x = p_x;
            y = p_y;
        }
        GridKey() {}
    };

    union CellKey {
        uint32_t id = 0;
        struct {
            GridKey octant;
            GridKey cell;
        };

        static uint32_t hash(const CellKey &p_key) {
			return p_key.id;
		}
		_FORCE_INLINE_ bool operator<(const CellKey &p_key) const {
			return id < p_key.id;
		}
		_FORCE_INLINE_ bool operator==(const CellKey &p_key) const {
			return id == p_key.id;
		}

        CellKey() {}
        CellKey(uint32_t p_id) {
            id = p_id;
        }
        CellKey(const GridKey &p_octant, const GridKey &p_cell) {
            octant = p_octant;
            cell = p_cell;
        }
    };

    struct Cell {
        uint8_t status = CELL_EMPTY;
        Vector2 position;
        CellKey key;
        Cell *previous = nullptr;
        real_t g_score = 0;
		real_t f_score = 0;
        uint64_t open_pass = 0;
		uint64_t closed_pass = 0;
    };

    struct SortCells {
		_FORCE_INLINE_ bool operator()(const Cell *A, const Cell *B) const { // Returns true when the Point A is worse than Point B.
			if (A->f_score > B->f_score) {
				return true;
			} else if (A->f_score < B->f_score) {
				return false;
			} else {
				return A->g_score < B->g_score; // If the f_costs are the same then prioritize the points that are further away from the start.
			}
		}
	};

    struct Octant {
        HashMap<GridKey, Cell *> cells;
        bool active = false;
    };

    int max_agent_cells = 6; // Maximum number of cells an agent can occupy.
    real_t cell_size = 1.0f; // Size of each cell in the grid.
    int octant_size = 16; // Size of the octant in cells.
    int octant_lower = -8;
    int octant_upper = 7;
    bool has_gridmap = false;
    Transform3D transform;
    real_t octant_length = 0.0f; // Length of the octant in world units.

    HashMap<GridKey, Octant *, GridKey> octants;
    // HashMap<CellKey, Cell *, CellKey> cell_map;
    uint64_t pass = 0;
    Cell *last_closest_cell = nullptr;

    bool _solve(Cell *p_start, Cell *p_end, int p_clearance);
    float _estimate_cost(const Cell *p_from, const Cell *p_to) const;
    float _compute_cost(const Cell *p_from, const Cell *p_to) const;
    
protected:
    uint32_t local_to_cell(const Vector3 &p_local_position) const;
    _FORCE_INLINE_ Vector2 cell_key_to_local(const CellKey &p_cell_key) const;
    
    void set_up_grid(float p_cell_size, int p_octant_size, const Transform3D &p_transform);
    void set_transform(const Transform3D &p_transform);

	static void _bind_methods();
    
public:
    TypedArray<Vector2i> get_path(uint32_t p_start, uint32_t p_end, int p_clearance) const;
    void clear();
    void add_octant(const Vector2i &p_octant_coord, const TypedArray<Vector2i> &p_cells);
    void remove_octant(const Vector2i &p_octant_coord);
    // void hide_octant(const Vector2i &p_octant_coord);
    // void show_octant(const Vector2i &p_octant_coord, PackedByteArray &p_cache);
    void generate_grid();

    void set_max_agent_cells(int p_max_cells);
    int get_max_agent_cells() const;
    float get_cell_size() const;
    int get_octant_size() const;

    PackedStringArray get_configuration_warnings() const override;

	NavGrid();
    ~NavGrid();
};

#endif // NAVGRID_H