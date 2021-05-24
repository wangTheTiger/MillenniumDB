#include "path_manager.h"

#include <new>         // placement new
#include <type_traits> // aligned_storage

// memory for the object
static typename std::aligned_storage<sizeof(PathManager), alignof(PathManager)>::type path_manager_buf;
// global object
PathManager& path_manager = reinterpret_cast<PathManager&>(path_manager_buf);

PathManager::PathManager(QuadModel& model) :
    model (model) { }


void PathManager::init(QuadModel& model) {
    new (&path_manager) PathManager(model); // placement new
}


ObjectId PathManager::set_path(const SearchState* visited_pointer, uint_fast32_t property_path_id) {
    //saved_visited_pointer = visited_pointer;
    // TODO: Refactor visited pointer allocation
    auto path_id = paths.size();
    paths.push_back(visited_pointer);
    return ObjectId(GraphModel::VALUE_PATH_MASK | path_id);
}


void PathManager::print(std::ostream& os, uint64_t path_id) const {
    //auto current_state = saved_visited_pointer; // TODO: set from path_id
    auto current_state = paths[path_id];
    std::vector<std::string> path_string;
    std::vector<bool> directions;
    while (current_state != nullptr) {
       std::stringstream ss;
       ss << model.get_graph_object(current_state->object_id);
       path_string.push_back(ss.str());
       directions.push_back(current_state->direction);
       current_state = const_cast<SearchState*>(current_state->previous);
    }
    os << path_string[path_string.size() - 1];
    // TODO: Crear un puntero al automata. Añadir labels al print, manejar invertidos
    for (int i = path_string.size() - 2; i >= 0; i--) {
        if (directions[i]) {
            os << "<=" << path_string[i];
        } else {
            os << "=>" << path_string[i];
        }
    }
}