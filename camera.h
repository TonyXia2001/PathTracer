#ifndef CAMERA_H
#define CAMERA_H
#include "common.h"

#include "color.h"
#include "hittable.h"

class camera {
    public:
        double  aspect_ratio      = 1.0;
        int     image_width       = 100;
        int     samples_per_pixel = 10;

        void render(const hittable& world) {
            initialize();

            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

            for (int j = 0; j < image_height; j++) {
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0,0,0);
                    for (int k = 0; k < samples_per_pixel; k++) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, world);
                    }
                    write_color(std::cout, pixel_color, samples_per_pixel);
                }
            }

            std::clog << "\rDone.                             \n";
        }

    private:
        int     image_height;   // rendered image height
        point3  center;         // camera center in world space
        point3  pixel00_loc;    // location of pixel 0, 0
        vec3    pixel_delta_u;  // width of each pixel
        vec3    pixel_delta_v;  // height of each pixel

        void initialize() {
            image_height = static_cast<int> (image_width / aspect_ratio);
            image_height = std::max(image_height, 1);
            center = point3(0,0,0);

            // viewport
            double focal_length = 1.0;
            double viewport_height = 2.0;
            double viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);
            vec3 viewport_u = vec3(viewport_width, 0, 0);
            vec3 viewport_v = vec3(0, -viewport_height, 0);
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            point3 viewport_upper_left = center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
        }

        ray get_ray(int i, int j) const {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto pixel_sample = pixel_center + pixel_sample_square();
            auto ray_origin = center;
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        vec3 pixel_sample_square() const {
            auto px = -0.5 + random_double();
            auto py = -0.5 + random_double();
            return (px * pixel_delta_u) + (py * pixel_delta_v);
        }

        color ray_color(const ray& r, const hittable& world) const {
            hit_record rec;
            if (world.hit(r, interval(0, infinity), rec)) {
                return 0.5 * (rec.normal + color(1, 1, 1));
            }

            vec3 unit_direction = unit_vector(r.direction());
            double a = 0.5*(unit_direction.y() + 1.0);
            return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
        }
};

#endif

