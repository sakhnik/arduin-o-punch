// Global resolution
//$fs = 0.1;  // Don't generate smaller facets than 0.1 mm
//$fa = 5;    // Don't generate larger angles than 5 degrees

edge_rounding = 1.5;
corner_radius = 4;

bottom_length = 124.3;
bottom_width = 71.9;
top_length = 120;
top_width = 67.9;
height = 37.2;

module corner(rCorner, rEdge) {
    // One corner of the box: a series of spheres rotated from 0° to 90°
    for (angle = [0 : $fa : 90]) {
        rotate(angle) translate([rCorner, 0, 0]) sphere(rEdge);
    }
}

module semiFace(dx, dy, rCorner, rEdge) {
    // Two corners
    translate([dx, dy, 0]) corner(rCorner, rEdge);
    translate([-dx, dy, 0]) rotate([0, 0, 90]) corner(rCorner, rEdge);
}

module face(length, width, rCorner, rEdge) {
    // Four corners with everything in between them.
    dx = width / 2 - rCorner - rEdge;
    dy = length / 2 - rCorner - rEdge;

    hull() {
        semiFace(dx, dy, rCorner, rEdge);
        rotate([0, 0, 180]) semiFace(dx, dy, rCorner, rEdge);
        //cube([dx + dx, dy + dy, 2 * rEdge], center=true);
    }
}

module box_z77() {
    color("DimGrey", 1.0) {
        hull() {
            face(bottom_length, bottom_width, corner_radius, 2 * edge_rounding);
            translate([0, 0, height - 2 * edge_rounding])
                face(top_length, top_width, corner_radius, edge_rounding);
        }
    }
}

module button() {
    color("DimGrey") {
        // Base
        cylinder(h=1, r=7);
        // Slanted part
        translate([0, 0, 1]) {
            // Sliced cylinder
            difference() {
                cylinder(7, 7);
                translate([0, 0, 2]) cylinder(7, 7);
            }
        }
    }

    // Button
    color("Red") {
        cylinder(h=6, r=4.5);
    }
}

module plug() {
    difference() {
        union() {
            color("DimGrey") {
                cylinder(h=2, r=4.5);
            }
            color("LightGrey") {
                cylinder(h=2.5, r=3.5);
            }
        }
        cylinder(h=4, r=5.5/2);
    }
    color("LightGrey") {
        cylinder(h=2, r=2.1*0.5);
    }
}

module programmator_pin() {
    cube([5, 0.64, 0.64], center=true);
}

module programmator_connector() {
    for (pin = [0 : 1 : 5]) {
        translate([0, pin * 2.54, 0]) programmator_pin();
    }
    translate([0, 2.54 * 2.5, 0]) {
        difference() {
            cube([5, 2.54 * 6 + 0.2, 2.54 + 0.2], center=true);
            cube([5, 2.54 * 6, 2.54], center=true);
        }
    }
}

front_slope = 0.5 * (bottom_length - top_length) / height;

module station() {
    // Elevate slightly to align with z=0
    translate([0, 0, edge_rounding * 2]) {

        // Box with programmator connector
        difference() {
            box_z77();
            translate([bottom_width * 0.5 + 1, bottom_length * 0.125, 2.54]) {
                programmator_connector();
            }
        }

        // Position the button
        translate([0, -(top_length + bottom_length) / 4 + height * 0.25 * front_slope, height / 2]) {
            rotate([90 - atan(front_slope), 0, 0])
                button();
        }

        // Position the plug
        translate([-top_width / 4, -(top_length + bottom_length) / 4 + height * 0.25 * front_slope, height / 2]) {
            rotate([90 - atan(front_slope), 0, 0])
                plug();
        }

        // Position the led
        color("Red") {
            translate([0, top_length * 0.125, height - 2]) {
                sphere(1.5);
            }
        }
    }
}

station();
