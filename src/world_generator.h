#pragma once

#include <godot_cpp/classes/node.hpp>

#include "world.h"

namespace godot {

class GDC_WorldGenerator: public Node {
    GDCLASS(GDC_WorldGenerator, Node)

public:
    void _ready() override;

    void set_world(GDC_World *p_world);
    GDC_World *get_world() const;

protected:
    static void _bind_methods();

private:
    GDC_World *world = nullptr;

    void generate();
};

} // namespace godot
