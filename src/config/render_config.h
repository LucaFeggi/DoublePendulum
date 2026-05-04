#ifndef CONFIG_RENDER_CONFIG_H
#define CONFIG_RENDER_CONFIG_H

// ----- Rendering parameters -----

#define COLOR_DECAY_PER_MILLE 995
#define COLOR_DECAY ((double)COLOR_DECAY_PER_MILLE / 1000.0)
#define COLOR_DECAY_REFERENCE_FPS 60 // apply the equivalent of COLOR_DECAY_PER_MILLE every 1/60th of a second

#define ROD_WIDTH_PER_MILLE 2000
#define ROD_WIDTH_PIXELS ((float)ROD_WIDTH_PER_MILLE / 1000.0f)

#define TRAIL 1
#define TRAIL_WIDTH_PER_MILLE 1500
#define TRAIL_WIDTH_PIXELS ((float)TRAIL_WIDTH_PER_MILLE / 1000.0f)

// Trail history is split into render-target buckets. New trail segments are
// drawn into the current bucket; old buckets are rendered with age-based alpha
// and eventually cleared, so stale trail pixels disappear completely.
#define TRAIL_DURATION_MILLISECONDS 2500
#define TRAIL_BUCKET_MILLISECONDS 100
#define TRAIL_DURATION_SECONDS ((float)TRAIL_DURATION_MILLISECONDS / 1000.0f)
#define TRAIL_BUCKET_SECONDS ((float)TRAIL_BUCKET_MILLISECONDS / 1000.0f)
#define TRAIL_BUCKET_COUNT ((TRAIL_DURATION_MILLISECONDS + TRAIL_BUCKET_MILLISECONDS - 1) / TRAIL_BUCKET_MILLISECONDS)

// > 1 fades older buckets more aggressively while keeping the newest trail bright.
#define TRAIL_FADE_GAMMA_PER_MILLE 1350
#define TRAIL_FADE_GAMMA ((float)TRAIL_FADE_GAMMA_PER_MILLE / 1000.0f)

#endif // CONFIG_RENDER_CONFIG_H
