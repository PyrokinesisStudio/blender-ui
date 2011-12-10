import bpy
settings = bpy.context.edit_movieclip.tracking.settings

settings.default_tracker = 'Hybrid'
settings.default_pyramid_levels = 2
settings.default_correlation_min = 0.7
settings.default_pattern_size = 31
settings.default_search_size = 300
settings.default_frames_limit = 0
settings.default_pattern_match = 'PREV_FRAME'
settings.default_margin = 5
