\
    #define _CRT_SECURE_NO_WARNINGS
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <string.h>
    #include <math.h>
    #include <time.h>

    typedef struct { int x, z; } I2;
    typedef struct {
        uint64_t seed;
        int spawn_x, spawn_z;
        int jx, jz;    double dj;
        int bx, bz;    double db;
        int ox, oz;    double do_;
    } HitRow;

    typedef struct {
        int radius;
        int target_hits;
        uint64_t max_seeds;
        int mc_major, mc_minor;
        uint64_t start_seed;
        uint64_t end_seed;
        const char* out_csv;
        bool allow_deep_warm;
        bool verify_one;
        uint64_t verify_seed;
    } Config;

    static I2* make_disk_offsets(int R, int* out_count) {
        if (R <= 0) { *out_count = 0; return NULL; }
        size_t cap = (size_t)(M_PI * R * R) + 64;
        I2* pts = (I2*)malloc(cap * sizeof(I2));
        if (!pts) { perror("malloc"); exit(1); }
        size_t n = 0;
        int R2 = R * R;
        for (int dz = -R; dz <= R; ++dz) {
            int dz2 = dz*dz;
            int dx_max = (int)floor(sqrt((double)(R2 - dz2)));
            for (int dx = -dx_max; dx <= dx_max; ++dx) {
                if (n >= cap) {
                    cap = cap*3/2 + 32;
                    pts = (I2*)realloc(pts, cap * sizeof(I2));
                    if (!pts) { perror("realloc"); exit(1); }
                }
                pts[n++] = (I2){ dx, dz };
            }
        }
        *out_count = (int)n;
        return pts;
    }

    static FILE* csv_open(const char* path) {
        FILE* f = fopen(path, "w");
        if (!f) { perror("fopen"); exit(1); }
        fprintf(f,
          "seed,spawn_x,spawn_z,"
          "jungle_x,jungle_z,dist_jungle,"
          "badlands_x,badlands_z,dist_badlands,"
          "warm_ocean_x,warm_ocean_z,dist_warm_ocean\n");
        fflush(f);
        return f;
    }
    static void csv_write(FILE* f, const HitRow* r) {
        fprintf(f, "%llu,%d,%d,%d,%d,%.3f,%d,%d,%.3f,%d,%d,%.3f\n",
            (unsigned long long)r->seed,
            r->spawn_x, r->spawn_z,
            r->jx, r->jz, r->dj,
            r->bx, r->bz, r->db,
            r->ox, r->oz, r->do_);
        fflush(f);
    }

    /* ---- Cubiomes glue: replace with real calls ---- */

    static void* cb_init_generator(int mc_major, int mc_minor, uint64_t seed) {
        (void)mc_major; (void)mc_minor; (void)seed;
        return (void*)0x1; // stub non-NULL
    }

    static bool cb_get_spawn(void* gen, int* out_x, int* out_z) {
        (void)gen;
        *out_x = 0; *out_z = 0; 
        return true;
    }

    typedef struct { bool is_jungle, is_badlands, is_warm_ocean, is_deep_warm_ocean; } BiomeFlags;
    static BiomeFlags cb_query_surface_biome(void* gen, int x, int z) {
        (void)gen; (void)x; (void)z;
        BiomeFlags b = {0};
        return b;
    }

    static inline double d2d(int x1, int z1, int x2, int z2) {
        double dx = (double)x1 - (double)x2;
        double dz = (double)z1 - (double)z2;
        return sqrt(dx*dx + dz*dz);
    }

    static void die_usage(const char* msg) {
        if (msg) fprintf(stderr, "Error: %s\n", msg);
        fprintf(stderr,
          "Usage:\n"
          "  seedscan --mc 1.21 --radius 100 --target 20 --max-seeds 500000000 "
          "[--start-seed N] [--end-seed M] [--out results.csv] [--allow-deep-warm] "
          "[--verify SEED]\n");
        exit(2);
    }

    static Config parse_args(int argc, char** argv) {
        Config c = {0};
        c.radius = 100;
        c.target_hits = 20;
        c.max_seeds = 500000000ULL;
        c.mc_major = 1; c.mc_minor = 21;
        c.start_seed = 0;
        c.end_seed = 0;
        c.out_csv = "results.csv";
        c.allow_deep_warm = false;
        c.verify_one = false;

        for (int i = 1; i < argc; ++i) {
            if (!strcmp(argv[i], "--mc")) {
                if (i+1 >= argc) die_usage("--mc needs value");
                if (sscanf(argv[++i], "%d.%d", &c.mc_major, &c.mc_minor) != 2)
                    die_usage("bad --mc format (use e.g. 1.21)");
            } else if (!strcmp(argv[i], "--radius")) {
                if (i+1 >= argc) die_usage("--radius needs value");
                c.radius = atoi(argv[++i]);
            } else if (!strcmp(argv[i], "--target")) {
                if (i+1 >= argc) die_usage("--target needs value");
                c.target_hits = atoi(argv[++i]);
            } else if (!strcmp(argv[i], "--max-seeds")) {
                if (i+1 >= argc) die_usage("--max-seeds needs value");
                c.max_seeds = strtoull(argv[++i], NULL, 10);
            } else if (!strcmp(argv[i], "--start-seed")) {
                if (i+1 >= argc) die_usage("--start-seed needs value");
                c.start_seed = strtoull(argv[++i], NULL, 10);
            } else if (!strcmp(argv[i], "--end-seed")) {
                if (i+1 >= argc) die_usage("--end-seed needs value");
                c.end_seed = strtoull(argv[++i], NULL, 10);
            } else if (!strcmp(argv[i], "--out")) {
                if (i+1 >= argc) die_usage("--out needs value");
                c.out_csv = argv[++i];
            } else if (!strcmp(argv[i], "--allow-deep-warm")) {
                c.allow_deep_warm = true;
            } else if (!strcmp(argv[i], "--verify")) {
                if (i+1 >= argc) die_usage("--verify needs a seed");
                c.verify_one = true;
                c.verify_seed = strtoull(argv[++i], NULL, 10);
            } else {
                die_usage("unknown flag");
            }
        }
        if (c.radius <= 0) die_usage("radius must be > 0");
        if (c.target_hits <= 0) die_usage("target must be > 0");
        if (c.max_seeds == 0) die_usage("max-seeds must be > 0");
        if (c.end_seed && c.end_seed <= c.start_seed)
            die_usage("end-seed must be > start-seed");
        return c;
    }

    static bool check_seed(Config* cfg, uint64_t seed, HitRow* out) {
        void* gen = cb_init_generator(cfg->mc_major, cfg->mc_minor, seed);
        if (!gen) return false;

        int sx = 0, sz = 0;
        if (!cb_get_spawn(gen, &sx, &sz)) return false;

        static I2* disk = NULL; static int diskN = 0; static int cachedR = -1;
        if (cfg->radius != cachedR) {
            if (disk) free(disk);
            disk = make_disk_offsets(cfg->radius, &diskN);
            cachedR = cfg->radius;
        }

        bool haveJ=false, haveB=false, haveO=false;
        int jx=0,jz=0, bx=0,bz=0, ox=0,oz=0;
        double dj=0., db=0., do_=0.;

        for (int i=0;i<diskN;i++) {
            int x = sx + disk[i].x;
            int z = sz + disk[i].z;

            BiomeFlags b = cb_query_surface_biome(gen, x, z);

            if (!haveJ && b.is_jungle) {
                haveJ = true; jx = x; jz = z; dj = d2d(x,z,sx,sz);
            }
            if (!haveB && b.is_badlands) {
                haveB = true; bx = x; bz = z; db = d2d(x,z,sx,sz);
            }
            bool oceanHit = b.is_warm_ocean || (cfg->allow_deep_warm && b.is_deep_warm_ocean);
            if (!haveO && oceanHit) {
                haveO = true; ox = x; oz = z; do_ = d2d(x,z,sx,sz);
            }
            if (haveJ && haveB && haveO) {
                out->seed = seed;
                out->spawn_x = sx; out->spawn_z = sz;
                out->jx = jx; out->jz = jz; out->dj = dj;
                out->bx = bx; out->bz = bz; out->db = db;
                out->ox = ox; out->oz = oz; out->do_ = do_;
                return true;
            }
        }
        return false;
    }

    static int run_verify(Config* cfg) {
        HitRow r;
        bool ok = check_seed(cfg, cfg->verify_seed, &r);
        if (!ok) {
            printf("Seed %llu: NO PASS within R=%d\n",
                (unsigned long long)cfg->verify_seed, cfg->radius);
            return 1;
        }
        printf("Seed %llu: spawn=(%d,%d)\n", (unsigned long long)r.seed, r.spawn_x, r.spawn_z);
        printf("  Jungle     at (%d,%d), d=%.3f\n", r.jx, r.jz, r.dj);
        printf("  Badlands   at (%d,%d), d=%.3f\n", r.bx, r.bz, r.db);
        printf("  WarmOcean  at (%d,%d), d=%.3f\n", r.ox, r.oz, r.do_);
        return 0;
    }

    int main(int argc, char** argv) {
        Config cfg = parse_args(argc, argv);

        if (cfg.verify_one) {
            return run_verify(&cfg);
        }

        FILE* f = csv_open(cfg.out_csv);
        int found = 0;
        uint64_t checked = 0;
        const time_t t0 = time(NULL);

        uint64_t seed = cfg.start_seed;
        for (; ; ++seed) {
            if (cfg.end_seed && seed >= cfg.end_seed) break;
            if (checked >= cfg.max_seeds) break;

            HitRow row;
            if (check_seed(&cfg, seed, &row)) {
                csv_write(f, &row);
                found++;
                fprintf(stderr, "[%d/%d] seed=%llu\n", found, cfg.target_hits, (unsigned long long)seed);
                if (found >= cfg.target_hits) break;
            }

            checked++;
            if ((checked % 10000000ULL) == 0ULL) {
                time_t now = time(NULL);
                double dt = difftime(now, t0);
                double sps = dt > 0 ? (double)checked / dt : 0.0;
                fprintf(stderr, "Checked=%llu  Found=%d  (%.0f seeds/s)\n",
                        (unsigned long long)checked, found, sps);
            }
        }

        fclose(f);
        if (found < cfg.target_hits) {
            fprintf(stderr, "Stopped early: found %d/<%d> after %llu seeds.\n",
                    found, cfg.target_hits, (unsigned long long)checked);
            return 2;
        }
        fprintf(stderr, "Done: found %d after %llu seeds.\n",
                found, (unsigned long long)checked);
        return 0;
    }
