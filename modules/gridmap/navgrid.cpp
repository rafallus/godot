#include "navgrid.h"

TypedArray<Vector2i> NavGrid::get_path(uint32_t p_start, uint32_t p_end, int p_clearance) const {
    CellKey start_key = CellKey(p_start);
    CellKey end_key = CellKey(p_end);

    if (start_key == end_key) {
        Vector<Vector2> ret = { cell_key_to_local(start_key) };
        return ret;
    }

    Octant *start_octant = octants[start_key.octant];
    Octant *end_octant = start_key.octant == end_key.octant ? start_octant : octants[end_key.octant];
    Cell *start_cell = start_octant->cells[start_key.cell];
    Cell *end_cell = end_octant->cells[end_key.cell];
}

void NavGrid::clear() {
    for (KeyValue<GridKey, Octant *> &E : octants) {
        memdelete(E.value);
    }
    octants.clear();
}

void NavGrid::add_octant(const Vector2i &p_octant_coord, const TypedArray<Vector2i> &p_cells) {
    DEV_ASSERT(p_octant_coord.x >= -MAX_GRID_COORD && p_octant_coord.x < MAX_GRID_COORD);
    DEV_ASSERT(p_octant_coord.y >= -MAX_GRID_COORD && p_octant_coord.y < MAX_GRID_COORD);
    Octant *octant = memnew(Octant);
    octant->active = true;
    GridKey octant_key = GridKey(p_octant_coord);
    octants[octant_key] = octant;
    
    for (int i = 0; i < p_cells.size(); ++i) {
        Vector2i cell_coord = p_cells[i];
        DEV_ASSERT(cell_coord.x >= -MAX_GRID_COORD && cell_coord.x < MAX_GRID_COORD);
        DEV_ASSERT(cell_coord.y >= -MAX_GRID_COORD && cell_coord.y < MAX_GRID_COORD);
        CellKey cell_key;
        cell_key.octant = octant_key;
        cell_key.cell = GridKey(cell_coord);
        // DEV_ASSERT(!cell_map.has(cell_key));
        Cell *cell = memnew(Cell);
        // cell_map[cell_key] = cell;
        octant->cells[cell_key.cell] = cell;
    }
}

void NavGrid::remove_octant(const Vector2i &p_octant_coord) {
    GridKey octant_key = GridKey(p_octant_coord);
    ERR_FAIL_COND_MSG(!octants.has(octant_key), "Trying to remove a NavGrid octant, but it was not found.");
    Octant *octant = octants[octant_key];
    octant->active = false;
    octants.erase(octant_key);

    for (KeyValue<GridKey, Cell *> &E : octant->cells) {
        CellKey cell_key;
        cell_key.octant = octant_key;
        cell_key.cell = E.key;
        // DEV_ASSERT(cell_map.has(cell_key));
        E.value->status = CELL_REMOVED;
        // cell_map.erase(cell_key);
    }
}

// void NavGrid::hide_octant(const Vector2i &p_octant_coord) { //, PackedByteArray &r_cache) {
//     Octant *octant = octants.getptr(p_octant_coord);
//     ERR_FAIL_NULL_MSG(octant, "Trying to hide a NavGrid octant, but it was not found.");
//     GridKey octant_key = GridKey(p_octant_coord);
//     octant->active = false;
//     bool save_occupied = octant->cells.size() < (uint32_t)(octant_size * octant_size * 0.6f);
//     uint16_t ncells = save_occupied ? octant->cells.size() : octant_size * octant_size - octant->cells.size();
//     // r_cache.resize(ncells * 2 + 3);
//     // uint8_t *w = r_cache.ptrw();
// 	// *((uint8_t *)&w[0]) = (uint8_t)(save_occupied);
//     // *((uint8_t *)&w[1]) = ncells & 0xFF;
//     // *((uint8_t *)&w[2]) = (ncells >> 8) & 0xFF;
//     // int offset = 3;

//     for (KeyValue<GridKey, Cell> &E : octant->cells) {
//         CellKey cell_key;
//         cell_key.octant = octant_key;
//         cell_key.cell = GridKey(E.key);
//         E.value.status = CELL_HIDDEN;
//         cell_map.erase(cell_key);
//     }

//     octant->cells.clear();
// }

// void show_octant(const Vector2i &p_octant_coord, PackedByteArray &p_cache);

float NavGrid::get_cell_size() const {
    return cell_size;
}

void NavGrid::set_max_agent_cells(int p_max_cells){
    max_agent_cells = p_max_cells; 
}

int NavGrid::get_max_agent_cells() const {
    return max_agent_cells; 
}

int NavGrid::get_octant_size() const {
    return octant_size;
}

uint32_t NavGrid::local_to_cell(const Vector3 &p_local_position) const {
    CellKey cell_key;
    cell_key.octant.x = (int)(Math::round(p_local_position.x / octant_length));
    cell_key.octant.y = (int)(Math::round(p_local_position.z / octant_length));
    real_t x0 = p_local_position.x - cell_key.octant.x * octant_length;
    real_t y0 = p_local_position.z - cell_key.octant.y * octant_length;
    cell_key.cell.x = (int)(Math::round(x0 / cell_size));
    cell_key.cell.y = (int)(Math::round(y0 / cell_size));
    return cell_key.id;
}

Vector2 NavGrid::cell_key_to_local(const CellKey &p_cell_key) const {
    real_t x = p_cell_key.cell.x * cell_size + p_cell_key.octant.x * octant_length;
    real_t y = p_cell_key.cell.y * cell_size + p_cell_key.octant.y * octant_length;
    return Vector2(x, y);
}

void NavGrid::set_up_grid(float p_cell_size, int p_octant_size, const Transform3D &p_transform) {
    cell_size = p_cell_size;
    octant_size = p_octant_size;
    transform = p_transform;
    has_gridmap = true;
    octant_length = cell_size * octant_size;
    octant_lower = -octant_size / 2;
    octant_upper = octant_size / 2 - (1 - octant_size % 2);
}

void NavGrid::set_transform(const Transform3D &p_transform) {
    transform = p_transform;
    // TODO: Something to do here?
}

void NavGrid::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_max_agent_cells", "max_cells"), &NavGrid::set_max_agent_cells);
    ClassDB::bind_method(D_METHOD("get_max_agent_cells"), &NavGrid::get_max_agent_cells);
    ClassDB::bind_method(D_METHOD("get_cell_size"), &NavGrid::get_cell_size);
    ClassDB::bind_method(D_METHOD("get_octant_size"), &NavGrid::get_octant_size);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_agent_size", PROPERTY_HINT_RANGE, "2,32"), "set_max_agent_cells", "get_max_agent_cells");
}

PackedStringArray NavGrid::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

    if (!has_gridmap) {
        warnings.push_back(RTR("NavGrid must be a child node of a GridMap."));
    }

	return warnings;
}

bool NavGrid::_solve(Cell *p_start, Cell *p_end, int p_clearance) {
    bool found_route = false;
    last_closest_cell = nullptr;
    pass++;

	LocalVector<Cell *> open_list;
	SortArray<Cell *, SortCells> sorter;

	p_start->g_score = 0;
	p_start->f_score = _estimate_cost(p_start, p_end);
	// p_start->abs_g_score = 0;
	// p_start->abs_f_score = _estimate_cost(p_start->id, p_end->id);
	open_list.push_back(p_start);

	while (!open_list.is_empty()) {
		Cell *current = open_list[0];

		// Find point closer to end_point, or same distance to end_point but closer to begin_point.
		// if (last_closest_cell == nullptr || last_closest_point->abs_f_score > p->abs_f_score || (astar.last_closest_point->abs_f_score >= p->abs_f_score && astar.last_closest_point->abs_g_score > p->abs_g_score)) {
		// 	astar.last_closest_point = p;
		// }

		if (current == p_end) {
			found_route = true;
			break;
		}

		sorter.pop_heap(0, open_list.size(), open_list.ptr()); // Remove the current point from the open list.
		open_list.remove_at(open_list.size() - 1);
		current->closed_pass = pass; // Mark the point as closed.

        Octant *current_octant = octants[current->key.octant];
        GridKey octant_key = current->key.octant;
		for (int32_t ix = -1; ix <= 1; ++ix) {
            int32_t nx = current->key.cell.x + ix;
            if (nx < octant_lower) {
                nx = octant_upper;
                octant_key.x--;
            }
            if (nx > octant_upper) {
                nx = octant_lower;
                octant_key.x++;
            }
            for (int32_t iy = -1; iy <= 1; ++iy) {
                if (ix == 0 && iy == 0) {
                    continue;
                }
                int32_t ny = current->key.cell.y + iy;
                if (ny < octant_lower) {
                    ny = octant_upper;
                    octant_key.y--;
                }
                if (ny > octant_upper) {
                    ny = octant_lower;
                    octant_key.y++;
                }
                Octant *neighbor_octant = octant_key == current->key.octant ? current_octant : octants[octant_key];
                GridKey neighbor_key = GridKey(nx, ny);
                if (neighbor_octant->cells.has(neighbor_key)) {
                    Cell *neighbor_cell = neighbor_octant->cells[neighbor_key];
                    if (neighbor_cell->closed_pass == pass) {
                        continue;
                    }
                    real_t tentative_g_score = current->g_score + _compute_cost(current, neighbor_cell);
                    bool new_point = false;

                    if (neighbor_cell->open_pass != pass) { // The point wasn't inside the open list.
                        neighbor_cell->open_pass = pass;
                        open_list.push_back(neighbor_cell);
                        new_point = true;
                    } else if (tentative_g_score >= neighbor_cell->g_score) { // The new path is worse than the previous.
                        continue;
                    }

                    neighbor_cell->previous = current;
                    neighbor_cell->g_score = tentative_g_score;
                    neighbor_cell->f_score = neighbor_cell->g_score + _estimate_cost(neighbor_cell, p_end);
                    // neighbor_cell->abs_g_score = tentative_g_score;
                    // neighbor_cell->abs_f_score = e->f_score - e->g_score;

                    if (new_point) { // The position of the new points is already known.
                        sorter.push_heap(0, open_list.size() - 1, 0, neighbor_cell, open_list.ptr());
                    } else {
                        sorter.push_heap(0, open_list.find(neighbor_cell), 0, neighbor_cell, open_list.ptr());
                    }
                }
            }
		}
	}

	return found_route;
}

float NavGrid::_estimate_cost(const Cell *p_from, const Cell *p_to) const {
    //Manhatan distance heavily reduces the ammount of cells that are evaluated in open environments 
    return abs(p_from->position.x - p_to->position.x) + abs(p_from->position.y - p_to->position.y);
}

float NavGrid::_compute_cost(const Cell *p_from, const Cell *p_to) const {
    return p_from->position.distance_to(p_to->position);
}

NavGrid::NavGrid() {
}

NavGrid::~NavGrid() {
    clear();
}