include <station.scad>

module planck() {
    translate([0, 0, -15])
        cube([bottom_width, bottom_length * 2, 30], center = true);
}

holder_length = 20;
holder_thickness = 2;
holder_depth = 20;

holder_height = height + holder_depth + holder_thickness;

module holder() {
    color("Silver")
        translate([0, 0, holder_height / 2 - holder_depth])
        union() {
            // Overhead bracket
            difference() {
                cube([bottom_width + holder_thickness, holder_length, holder_height], center=true);
                cube([bottom_width, holder_length + 10, holder_height - holder_thickness], center=true);
            }
            translate([0, -holder_length * 0.5, holder_height * 0.5 - holder_depth + holder_thickness * 0.5])
                // Front plate
                difference() {
                    cube([bottom_width, holder_thickness, height], center=true);
                    // Hole for the switch
                    translate([0, holder_thickness, 0]) rotate([90, 0, 0]) cylinder(h = holder_thickness * 4, r = 2);
                    // Hole for the charger plug
                    translate([-bottom_width * 0.2, holder_thickness, 0]) rotate([90, 0, 0]) {
                        hull() {
                            cylinder(h = holder_thickness * 4, r = 5);
                            translate([-8, 0, 0]) cylinder(h = holder_thickness * 4, r = 5);
                        }
                    }
                }
            // Bottom plate (apron)
            translate([0, -holder_length, -holder_depth * 0.5 + holder_thickness])
                cube([bottom_width, holder_length, holder_thickness], center=true);
        }

}

planck();

translate([0, -bottom_length * 0.5 + corner_radius, 0])
    holder();

translate([0, bottom_length * 0.5 - 2 * corner_radius, 0])
    rotate([0, 0, 180]) holder();
