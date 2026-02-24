#include "gdexample.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GDExample::_bind_methods() {
}


void GDExample::_ready() {
    UtilityFunctions::print("Hello from C++ GDExtension!");
}