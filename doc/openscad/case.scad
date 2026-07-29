include <BOSL2/std.scad>

$fn = 90;

length = 105;
width = 80;
bottom_thickness = 4;
platform_thickness = 4;
lid_height = 25;
lid_thickness = 6;

battery_d = 19;
battery_h = 66;
battery_y = 30;

pcb_offset_y = 17;  // back
rfid_offset_y = 40; // fwd
rfid_offset_z = lid_height - lid_thickness;

inner_width = width - 2 * lid_thickness;
inner_length = length - 2 * lid_thickness;
inner_height = lid_height - lid_thickness;
screw_x = width * 0.5 - lid_thickness;
screw_y = length * 0.5 - lid_thickness;

module battery() {
    fwd(battery_y)
    cylinder(h=battery_h, r=battery_d/2, anchor=LEFT, orient=LEFT);
}

module pcb_model() {
    pcb_width = 46;
    pcb_length = 56;
    pcb_height = 7;
    pcb_hole_r = 0.75;
    pcb_hole_offset = 3;
    pcb_led_offset_y = 6;
    pcb_pad_size = 4;
    pcb_pad_offset = 8;
    pcb_wire_width = 3;

    hole_x = pcb_width * 0.5 - pcb_hole_offset;
    hole_y = pcb_length * 0.5 - pcb_hole_offset;
    pad_x = pcb_width * 0.5 - pcb_pad_offset;
    pad_y = pcb_length * 0.5 - pcb_pad_offset;

    difference() {
        union() {
            cuboid([pcb_width, pcb_length, pcb_height], anchor=BOTTOM);

            // hole for the LED
            fwd(pcb_length * 0.5 - pcb_led_offset_y)
                cylinder(h=lid_height, r=1.5, anchor=BOTTOM);

            // holes for the pad wires
            move([pad_x, -pcb_length * 0.5, 0])
                cuboid([pcb_wire_width, pcb_wire_width, pcb_height], anchor=BOTTOM+BACK);
            move([pcb_width * 0.5, -pad_y, 0])
                cuboid([pcb_wire_width, pcb_wire_width, pcb_height], anchor=BOTTOM+LEFT);

            // holes for the pads
            move([pad_x, -(pcb_length/2 - pcb_pad_size/2), 0])
                cylinder(h=lid_height, r=1, anchor=TOP);
            move([pcb_width/2 - pcb_pad_size/2, -pad_y, 0])
                cylinder(h=lid_height, r=1, anchor=TOP);
        }

        // holes
        for (dx = [-1:2:1]) {
            for (dy = [-1:2:1]) {
                move([dx * hole_x, dy * hole_y, 0])
                cylinder(h=10, r=pcb_hole_r);
            }
        }
    }
}

module pcb() {
    back(pcb_offset_y) pcb_model();
}

module rfid_model() {
    rfid_width = 40;
    rfid_length = 60;
    rfid_height = 5;

    rfid_hole_r = 1.5;
    rfid_hole1_offset = 6;
    rfid_hole2_offset_x = 2;
    rfid_hole2_offset_y = 15;

    hole1_x = rfid_width/2 - rfid_hole1_offset;
    hole1_y = rfid_length - rfid_hole1_offset;

    hole2_x = rfid_width/2 - rfid_hole2_offset_x;

    difference() {
        cuboid([rfid_width, rfid_length, rfid_height], anchor=TOP+BACK);

        move([hole1_x, -hole1_y, 0])
            cylinder(h=rfid_height+1, r=rfid_hole_r, anchor=TOP);
        move([-hole1_x, -hole1_y, 0])
            cylinder(h=rfid_height+1, r=rfid_hole_r, anchor=TOP);
        move([hole2_x, -rfid_hole2_offset_y, 0])
            cylinder(h=rfid_height+1, r=rfid_hole_r, anchor=TOP);
        move([-hole2_x, -rfid_hole2_offset_y, 0])
            cylinder(h=rfid_height+1, r=rfid_hole_r, anchor=TOP);
    }
}

module rfid() {
    move([0, rfid_offset_y, rfid_offset_z])
        rfid_model();
}

module screw_supports() {
    for (dx = [-1:2:1]) {
        for (dy = [-1:2:1]) {
            move([dx * screw_x, dy * screw_y, 0])
            cylinder(h=inner_height, r=5);
        }
    }
}

module lid_shell() {
    difference() {
        cuboid(
            [width, length, lid_height],
            rounding=5,
            edges=[FRONT+LEFT, FRONT+RIGHT, BACK+LEFT, BACK+RIGHT, TOP],
            anchor=BOTTOM
        );

        down(1)
        cuboid(
            [inner_width, inner_length, 1 + inner_height],
            rounding=5,
            edges=[FRONT+LEFT, FRONT+RIGHT, BACK+LEFT, BACK+RIGHT],
            anchor=BOTTOM
        );
    }
}

module outer_lid() {
    lid_shell();
    screw_supports();
    // battery+rfid support
    up(inner_height)
    cuboid([inner_width, inner_length, platform_thickness], anchor=TOP);
};

module screw_holes() {
    for (dx = [-1:2:1]) {
        for (dy = [-1:2:1]) {
            move([dx * screw_x, dy * screw_y, -bottom_thickness])
            cylinder(h=lid_height-lid_thickness+bottom_thickness, r=1);
        }
    }
}

module counter_sinks() {
    for (dx = [-1:2:1]) {
        for (dy = [-1:2:1]) {
            move([dx * screw_x, dy * screw_y, -bottom_thickness])
            cylinder(h=2, r1=3, r2=1, anchor=BOTTOM);
        }
    }

}

module lid() {
    difference() {
        outer_lid();

        screw_holes();
        battery();
        pcb();
        rfid();
    }
}

module base() {
    difference() {
        union() {
            cuboid(
                [width, length, bottom_thickness],
                rounding=5,
                edges=[FRONT+LEFT, FRONT+RIGHT, BACK+LEFT, BACK+RIGHT],
                anchor=TOP
            );

            cuboid(
                [inner_width, inner_length, platform_thickness],
                anchor=BOTTOM
            );
        }

        screw_supports();
        battery();
        pcb();

        down(bottom_thickness)
            screw_holes();
        counter_sinks();
    }
}

base();

right(100)
    lid();
