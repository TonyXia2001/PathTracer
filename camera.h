#ifndef CAMERA_H
#define CAMERA_H
#include "common.h"

#include "color.h"
#include "hittable.h"
#include "material.h"

class camera {
    public:
        double  aspect_ratio      = 1.0;
        int     image_width       = 100;
        int     samples_per_pixel = 10;
        int     max_depth         = 10;
        
        double  vfov              = 90; // vertical FOV
        point3  lookfrom          = point3(0, 0, -1);
        point3  lookat            = point3(0, 0, 0);
        vec3    vup               = vec3(0, 1, 0);
        
        double  defocus_angle     = 0;  // Variation angle of rays through each pixel
        double  focus_dist        = 10;    // Distance from camera lookfrom point to plane of perfect focus

        void render(const hittable& world) {
            initialize();

            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

            for (int j = 0; j < image_height; j++) {
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0,0,0);
                    for (int k = 0; k < samples_per_pixel; k++) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
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
        vec3    u, v, w;
        vec3    defocus_disk_u;
        vec3    defocus_disk_v;

        void initialize() {
            image_height = static_cast<int> (image_width / aspect_ratio);
            image_height = std::max(image_height, 1);
            center = lookfrom;

            // viewport
            double theta = degrees_to_radians(vfov);
            double viewport_height = tan(theta/2) * focus_dist * 2;
            double viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);

            vec3 viewport_u = viewport_width * u;
            vec3 viewport_v = viewport_height * -v;
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            point3 viewport_upper_left = center - focus_dist * w - viewport_u/2 - viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle) / 2);
            defocus_disk_u = defocus_radius * u;
            defocus_disk_v = defocus_radius * v;
        }

        ray get_ray(int i, int j) const {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto pixel_sample = pixel_center + pixel_sample_square();
            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        vec3 pixel_sample_square() const {
            auto px = -0.5 + random_double();
            auto py = -0.5 + random_double();
            return (px * pixel_delta_u) + (py * pixel_delta_v);
        }

        point3 defocus_disk_sample() const {
            // Returns a random point in the camera defocus disk.
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }

        color ray_color(const ray& r, int depth, const hittable& world) const {
            if (depth <= 0) return color(0,0,0);
            hit_record rec;
            // interval min is set to 0.001 in case the ray's starting point is below the surface due to rounding errors
            // shadow acne
            if (world.hit(r, interval(0.001, infinity), rec)) {
                ray scattered;
                color attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered)) {
                    return attenuation * ray_color(scattered, depth-1, world);
                }
                return color(0,0,0);
            }

            vec3 unit_direction = unit_vector(r.direction());
            double a = 0.5*(unit_direction.y() + 1.0);
            return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
        }
};

#endif

