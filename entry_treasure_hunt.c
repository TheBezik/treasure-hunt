typedef struct Sprite {
	Gfx_Image* image;
	Vector2 size;
} Sprite;

typedef enum SpriteID {
	SPRITE_NULL,
	SPRITE_PLAYER,
	SPRITE_ROCK,
	SPRITE_MINERAL_ROCK,
	SPRITE_SPIKE,
	SPRITE_MAX
} SpriteID;
Sprite sprites[SPRITE_MAX];
Sprite* get_sprite(SpriteID id) {
	if (id >= SPRITE_MAX) id = 0;
	return &sprites[id];
}

typedef enum EntityArchetype {
	ARCH_NULL,
	ARCH_PLAYER,
	ARCH_ROCK,
	ARCH_MINERAL_ROCK,
	ARCH_SPIKE
} EntityArchetype;

typedef struct Entity {
	bool is_valid;
	bool render_sprite;
	EntityArchetype arch;
	Vector2 pos;
	SpriteID sprite_id;
} Entity;

#define MAX_ENTITIY_COUNT 1024
typedef struct World {
	Entity entities[MAX_ENTITIY_COUNT];
} World;

World* world = 0;

Entity* create_entity() {
	Entity* found_entity = 0;
	 for (int i = 0; i < MAX_ENTITIY_COUNT; i++) {
		Entity* existing_entity = &world->entities[i];
		if (!existing_entity->is_valid) {
			found_entity = existing_entity;
			break;
		}
	 }
	 assert(found_entity, "No more free entities!");
	 found_entity->is_valid = true;
	 return found_entity;
}

void destroy_entity(Entity* entity) {
	memset(entity, 0, sizeof(Entity));
}

void setup_player(Entity* entity) {
	entity->arch = ARCH_PLAYER;
	entity->sprite_id = SPRITE_PLAYER;
}

void setup_rock(Entity* entity) {
	entity->arch = ARCH_ROCK;
	entity->sprite_id = SPRITE_ROCK;
}

void setup_mineral_rock(Entity* entity) {
	entity->arch = ARCH_MINERAL_ROCK;
	entity->sprite_id = SPRITE_MINERAL_ROCK;
}

void setup_spike(Entity* entity) {
	entity->arch = ARCH_SPIKE;
	entity->sprite_id = SPRITE_SPIKE;
}

int entry(int argc, char **argv) {
	window.title = STR("Treasure Hunt");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x090a14ff);

	world = alloc(get_heap_allocator(), sizeof(World));
	memset(world, 0, sizeof(World));

	sprites[SPRITE_PLAYER] = (Sprite) {
		.image = load_image_from_disk(STR("resources/player.png"), get_heap_allocator()), 
		.size = v2(22.0, 35.0)
	};
	sprites[SPRITE_ROCK] = (Sprite) {
		.image = load_image_from_disk(STR("resources/rock_0.png"), get_heap_allocator()), 
		.size = v2(12.0, 5.0)
	};
	sprites[SPRITE_MINERAL_ROCK] = (Sprite) {
		.image = load_image_from_disk(STR("resources/rock_1.png"), get_heap_allocator()), 
		.size = v2(18.0, 10.0)
	};
	sprites[SPRITE_SPIKE] = (Sprite) {
		.image = load_image_from_disk(STR("resources/spike_0.png"), get_heap_allocator()), 
		.size = v2(14.0, 26.0)
	};

	Entity* player_entity = create_entity();
	setup_player(player_entity);

	for (int i = 0; i < 10; i++) {
		Entity* entity = create_entity();
		setup_rock(entity);
		entity->pos = v2(get_random_float32_in_range(-200.0, 200.0), get_random_float32_in_range(-200.0, 200.0));
		get_random_float32();
	}

	for (int i = 0; i < 10; i++) {
		Entity* entity = create_entity();
		setup_mineral_rock(entity);
		entity->pos = v2(get_random_float32_in_range(-200.0, 200.0), get_random_float32_in_range(-200.0, 200.0));
		get_random_float32();
	}

	for (int i = 0; i < 10; i++) {
		Entity* entity = create_entity();
		setup_spike(entity);
		entity->pos = v2(get_random_float32_in_range(-200.0, 200.0), get_random_float32_in_range(-200.0, 200.0));
		get_random_float32();
	}

	float64 seconds_counter = 0.0;
	s32 frame_count = 0;

	float64 last_time = os_get_current_time_in_seconds();

	float zoom = 1.0f / 2.0f;

	while (!window.should_close) {
		reset_temporary_storage();

		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);

		draw_frame.view = m4_make_scale(v3(zoom, zoom, 1.0f));

		float64 now = os_get_current_time_in_seconds();
		float64 delta_t = now - last_time;
		last_time = now;
		
		os_update(); 

		for (int i = 0; i < MAX_ENTITIY_COUNT; i++) {
			Entity* entity = &world->entities[i];
			if (!entity->is_valid) continue;
			
			switch (entity->arch) {
			default: {
				Sprite* sprite = get_sprite(entity->sprite_id);
				
				Vector2 size = sprite->size;
				Matrix4 xform = m4_scalar(1.0);
				xform = m4_translate(xform, v3(entity->pos.x, entity->pos.y, 0));
				xform = m4_translate(xform, v3(sprite->size.x * -0.5, 0, 0));
				draw_image_xform(sprite->image, xform, sprite->size, COLOR_WHITE);
				break;
			}
			}
		}

		if (is_key_just_pressed(KEY_ESCAPE)) window.should_close = true;

		Vector2 input_axis = v2(0, 0);
		if (is_key_down('A')) input_axis.x -= 1.0;
		if (is_key_down('D')) input_axis.x += 1.0;
		if (is_key_down('S')) input_axis.y -= 1.0;
		if (is_key_down('W')) input_axis.y += 1.0;
		input_axis = v2_normalize(input_axis);
		
		player_entity->pos = v2_add(player_entity->pos, v2_mulf(input_axis, 100 * delta_t));
		
		gfx_update();
		seconds_counter += delta_t;
		frame_count++;
		if (seconds_counter > 1.0) {
			log("fps: %i", frame_count);
			seconds_counter = 0.0;
			frame_count = 0;
		}
	}

	return 0;
}