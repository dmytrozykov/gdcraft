#pragma once

#include <godot_cpp/classes/node.hpp>

namespace godot {

class GDExample : public Node {
	GDCLASS(GDExample, Node)

protected:
	static void _bind_methods();

public:
	void _ready() override;
};

} // namespace godot