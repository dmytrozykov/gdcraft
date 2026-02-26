#pragma once

#include <vector>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/string.hpp>

#include "block_data.h"
#include "block_set.h"

namespace godot {

class GDC_BlockRegistry : public Node {
    GDCLASS(GDC_BlockRegistry, Node)

    static GDC_BlockRegistry *singleton;

    Ref<GDC_BlockSet> block_set;
    std::vector<Ref<GDC_BlockData>> blocks_by_id;   // index 0 = block with id 1
    HashMap<String, Ref<GDC_BlockData>> blocks_by_name; // lowercase keys

protected:
    static void _bind_methods();

public:
    static GDC_BlockRegistry *get_singleton();

    GDC_BlockRegistry() = default;
    ~GDC_BlockRegistry() override = default;

    void _enter_tree() override;
    void _exit_tree() override;

    Ref<GDC_BlockSet> get_block_set() const;
    void set_block_set(const Ref<GDC_BlockSet> &p_block_set);

    Ref<GDC_BlockData> get_block_by_id(int32_t id) const;
    Ref<GDC_BlockData> get_block_by_name(const String &p_name) const;
    int32_t get_block_count() const;

private:
    void reload();
};

} // namespace godot
