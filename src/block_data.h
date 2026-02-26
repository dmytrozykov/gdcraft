#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class GDC_BlockData : public Resource {
	GDCLASS(GDC_BlockData, Resource)

	String block_name;
	Color color;
	int32_t id = 0;

protected:
	static void _bind_methods();

public:
	GDC_BlockData() = default;
	~GDC_BlockData() override = default;

	String get_block_name() const;
	void set_block_name(const String &p_name);

	Color get_color() const;
	void set_color(const Color &p_color);

	int32_t get_id() const;
	void set_id(int32_t p_id);
};

} // namespace godot
