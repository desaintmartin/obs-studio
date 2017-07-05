#include <obs-module.h>
#include <obs-source.h>
#include <obs.h>
#include <util/platform.h>

struct transform_data {
	obs_source_t                   *context;

	gs_effect_t                    *effect;
	gs_eparam_t                    *transform_param;
	gs_eparam_t                    *texture_width, *texture_height;

	float                          texwidth, texheight;
};

static const char *transform_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("TransformFilter");
}

static void transform_update(void *data, obs_data_t *settings)
{
	struct transform_data *filter = data;
}

static void transform_destroy(void *data)
{
	struct transform_data *filter = data;

	if (filter->effect) {
		obs_enter_graphics();
		gs_effect_destroy(filter->effect);
		obs_leave_graphics();
	}

	bfree(data);
}

static void *transform_create(obs_data_t *settings, obs_source_t *context)
{
	struct transform_data *filter =
		bzalloc(sizeof(struct transform_data));
	char *effect_path = obs_module_file("transform.effect");

	filter->context = context;

	obs_enter_graphics();

	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	if (filter->effect) {
		filter->texture_width = gs_effect_get_param_by_name(
			filter->effect, "texture_width");
		filter->texture_height = gs_effect_get_param_by_name(
			filter->effect, "texture_height");
}

	obs_leave_graphics();

	bfree(effect_path);

	if (!filter->effect) {
		transform_destroy(filter);
		return NULL;
	}

	transform_update(filter, settings);
	return filter;
}

static void transform_render(void *data, gs_effect_t *effect)
{
	struct transform_data *filter = data;

	if (!obs_source_process_filter_begin(filter->context, GS_RGBA,
				OBS_ALLOW_DIRECT_RENDERING))
		return;

	filter->texwidth =(float)obs_source_get_width(
			obs_filter_get_target(filter->context));
	filter->texheight = (float)obs_source_get_height(
			obs_filter_get_target(filter->context));

	gs_effect_set_float(filter->texture_width, filter->texwidth);
  gs_effect_set_float(filter->texture_height, filter->texheight);

	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

	UNUSED_PARAMETER(effect);
}

static obs_properties_t *transform_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	UNUSED_PARAMETER(data);
	return props;
}

static void transform_defaults(obs_data_t *settings)
{
}

struct obs_source_info transform_filter = {
	.id = "transform_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = transform_getname,
	.create = transform_create,
	.destroy = transform_destroy,
	.update = transform_update,
	.video_render = transform_render,
	.get_properties = transform_properties,
	.get_defaults = transform_defaults
};

