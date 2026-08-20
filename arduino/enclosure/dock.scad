include <BOSL2/std.scad>

$fn = 90;

module pcb() {

    pcb_length = 30;
    pcb_width = 15.2;

    cuboid([pcb_width, pcb_length, 10], anchor=BOTTOM);

    // usb
    fwd(pcb_length/2 + 2.5) up(0.5)
    cuboid([9.5, 9.5, 3.5], edges=[LEFT+BOTTOM,RIGHT+BOTTOM], rounding=1.4, anchor=TOP+FRONT);

    fwd(1.5) left(3.5)
    cuboid([8.2, 8.2, 4], anchor=TOP);

    back(5)
    cuboid([pcb_width, pcb_length - 10, 2], anchor=TOP);

    back(pcb_length/2)
    cuboid([20, 15, 7], anchor=BOTTOM);

    right(pcb_width/2) back(pcb_length/2-4)
    cuboid([2, 5, 16], anchor=TOP+RIGHT+BACK);
}

difference() {

    back(15)
    cuboid([40, 70, 16], rounding=1.4);

    fwd(2.5)
    pcb();

    left(4) back(55) up(3.5)
    zrot(45)
    cuboid(
        [80, 106, 100],
        rounding=5,
        edges=[FRONT+LEFT, FRONT+RIGHT, BACK+LEFT, BACK+RIGHT],
        anchor=BOTTOM
    );
}
