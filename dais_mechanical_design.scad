// CHECK BEFORE PRINTING:
// - Are all dimensions correct?
// - Is material thickness correct?
// - Is kerf correct?
$fn = 50;

mt = 3; // Material thickness
// Kerf tip: Keep fingers the "right" size and make the slots a big smaller.
kerf = 0.125;
explode = false;
kc = 2; // Knoll clearance

numFaces = 16; // Number of edges on base plate
baseRadius = 150;
diskRadius = baseRadius - 20;

// If we call ngon directly with the specified baseRadius our disk will be too small.
// We want baseRadius to be the distance from the center to the middle of one of the edges.
// Here we compute the radius we need to acheive that.
trueBaseRadius = baseRadius * (1 / cos(0.5 * (360 / numFaces)));
trueDiskRadius = diskRadius * (1 / cos(0.5 * (360 / numFaces)));

wallUpper = 80; // Upper height of wall
wallLower = 60; // Lower hight of wall
wallWidth = norm([trueBaseRadius - mt, 0] - [(trueBaseRadius - mt) * cos(360 / numFaces), (trueBaseRadius- mt) * sin(360 / numFaces)]);
wallMiddleWidth = 0.8 * wallWidth;
wallSupportWidth = 30;
cutoutHeight = 1 - wallSupportWidth / wallUpper;
cutoutWidth = 5 * (1 / numFaces);
wireRadius = 1.5;
mountHeight = mt;
routingHoleRadius = 20;

function ex(enable) = (enable && explode) ? explodeAmount : 0;
function cutout(x, w, h) = x < w ? 1 - h * sin(180 * (x / w)) : 1;

module mount(tool = false, knoll = false)
{
    module mountInner()
    {
        union()
        {
            translate([-(baseRadius - mt), 0, 0])
            translate([-mt, -0.5 * 0.5 * wallWidth, 0])
            cube([mt, 0.5 * wallWidth, mt]);

            translate([baseRadius - mt, 0, 0])
            translate([0, -0.5 * 0.5 * wallWidth, 0])
            cube([mt, 0.5 * wallWidth, mt]);

            translate([-(baseRadius - mt), -0.5 * wallWidth, 0])
                cube([2 * baseRadius - 2 * mt, wallWidth, mt]);
        }
    }
    
    if (tool)
    {
        w = 0.5 * wallWidth - 2 * kerf;
        h = mt - 2 * kerf;
        translate([0, 0, mountHeight])
        translate([0, 0.5 * (wallWidth - w), 0])
        translate([0, 0, 0.5 * (mt - h)])
            cube([mt, w, h]);

    }
    else
    {
        color("pink")
        
        if (knoll)
        {
            translate([(trueBaseRadius + mt), 0.5 * wallWidth, 0]) 
            mountInner();
        }
        else
        {
            rotate([0, 0, -2 * (360 / numFaces)])
            translate([0, 0, mountHeight])
                mountInner();
        }
    }
}
module ngon(numFaces, radius, height)
{
    p = [for (i = [0:numFaces-1]) [radius * cos(i * (360 / numFaces)), radius * sin(i * (360 / numFaces))]];
    echo(p);
    linear_extrude(height)
        polygon(p);
}

module wallSupport(tool = false, knoll = false)
{   
    e = ex(!tool);

    width = wallSupportWidth;
    height = width;
    k = tool ? kerf : 0;
    fingerLength = (width * 0.5) - (tool ? 2 * kerf : 0);
    fingerWidth = mt - (tool ? 2 * kerf : 0);
    fingerHeight = mt;
    
    module finger()
    {
        translate([0, 0.5 * (mt - fingerWidth), 0]) // Center on part y axis (in case this is a tool)
        translate([-fingerLength - 0.5 * (width - fingerLength), 0, 0]) // Center on part x axis
        translate([0, 0, -fingerHeight]) // Flush with x axis
        cube([fingerLength, fingerWidth, fingerHeight]);
    }
    
    color("pink")
    union()
    {
        if (true)
        {
            rotate([-90, 0, 0]) // Turn upright
            linear_extrude(mt)
                polygon([[0, 0], [0, -height], [-width, 0]]);
        }
        
        finger();
        
        translate([fingerHeight, 0, 0])
        rotate([0, 90, 0])
            finger();
    }
}

module wall(a = wallUpper, b = wallUpper, slot = false, tool = false)
{
    if (tool)
    {
        
        translate([0, 0.5 * (wallWidth - (wallMiddleWidth - 2 * kerf)), wallLower])
                cube([mt, wallMiddleWidth - 2 * kerf, mt]);
    }
    else
    {
        difference()
        {
            union()
            {
                // Lower part
                cube([mt, wallWidth, wallLower + kerf]);
                
                // Middle part
                translate([0, 0.5 * (wallWidth - wallMiddleWidth), wallLower + kerf])
                    cube([mt, wallMiddleWidth, mt - 2 * kerf]);
                
                // Upper part
                translate([0, 0, wallLower + mt - kerf])
                translate([mt, 0, 0])
                rotate([0, -90, 0])
                linear_extrude(mt)
                polygon([
                    [0, 0],
                    [a, 0],
                    [b, wallWidth],
                    [0, wallWidth]]);
            }
            
            // Hole for wire
            if (a == b && a == wallUpper)
            {
                translate([0, 0, wallLower + mt - kerf + 0.5 * (a + b) - 2 * wireRadius])
                translate([0, 0.5 * wallWidth, 0])
                rotate([0, 90, 0]) // Rotate to correct orientation
                cylinder(mt, wireRadius, wireRadius);
            }
            
            translate([0, 0, wallLower + mt - kerf]) // Move to meet base plate
            translate([0, 0.5 * wallWidth, 0]) // Center on wall
            translate([0, -0.5 * mt, 0]) // Center on y-axis
                wallSupport(tool = true);
            
            if (slot)
            {
                mount(tool = true);
            }
        }
    }
}

module wallWithSupport(a, b, slot, tool = false, knoll = false)
{
    wall(a, b, slot, tool);
    
    if (tool)
    {
        translate([0, 0, wallLower + mt]) // Move to meet base plate
        translate([0, 0.5 * wallWidth, 0]) // Center on wall
        translate([0, -0.5 * mt, 0]) // Center on y-axis
            wallSupport(tool);
    }
    else
    {
        if (knoll)
        {
            translate([0, 0, wallLower + mt + max(a, b) + kc - kerf])
            translate([0, mt, mt])
            rotate([0, 0, -90])
                wallSupport(tool);
        }
        else
        {
            translate([0, 0, wallLower + mt - kerf]) // Move to meet base plate
            translate([0, 0.5 * wallWidth, 0]) // Center on wall
            translate([0, -0.5 * mt, 0]) // Center on y-axis
                wallSupport(tool);
        }
    }
}

module walls(tool = false, knoll = false)
{
    for (i = [0:numFaces-1])
    {
        a = i * (360 / numFaces);
        x1 = i / numFaces;
        x2 = (i+1)/ numFaces;
        echo(x1, x2);
        h1 = wallUpper * cutout(x1, cutoutWidth, cutoutHeight);
        h2 = wallUpper * cutout(x2, cutoutWidth, cutoutHeight);
        
        if (knoll)
        {
            translate([0, 0, mt])
            translate([0, i * (wallWidth + kc), 0])
            rotate([0, 90, 0])
                wallWithSupport(h1, h2, i == 6 || i == numFaces - 2, tool, knoll=true);
        }
        else
        {

            rotate([0, 0, a]) // Rotate to appropriate edge
            translate([-mt, 0, 0]) // Move flush to edge
            translate([baseRadius, 0, 0]) // Move to edge of base plate
            translate([0, -0.5 * wallWidth, 0]) // center on y axis
                wallWithSupport(h1, h2, i == 6 || i == numFaces - 2, tool);
        }
    }
}

module base()
{     
    color("green")
    
    difference()
    {
        rotate([0, 0, 0.5 * (360 / numFaces)])
            ngon(numFaces, trueBaseRadius, mt);
        
        translate([0, 0, -wallLower]) // Move down to collide with base
            walls(tool = true);
        
        translate([-0.3 * trueBaseRadius, -0.3 * trueBaseRadius, 0, ])
            cylinder(mt,routingHoleRadius, routingHoleRadius);
        
        // Cutouts for proximity sensor
        rotate([0, 0, -45])
        {
            translate([-4.5, 0, 0])
                cylinder(mt, 1.5, 1.5);
            translate([4.5, 0, 0])
                cylinder(mt, 1.5, 1.5);
        }
    }
}

module disk()
{
    rotate([0, 0, 0.5 * (360 / numFaces)])
    difference()
    {
        ngon(numFaces, trueDiskRadius, mt);
        
        for (i = [4:numFaces-1])
        {
            rotate([0, 0, i * (360 / numFaces)])
            translate([trueDiskRadius - 2 * wireRadius, 0, 0])
                cylinder(mt, wireRadius, wireRadius);
        }
    }
}

// The knoll flag enables layout of all the parts for laser cutting. Set to
// false to see a 3D model of the dais design.
knoll = true;

if (knoll)
{
    projection()
    {
        walls(knoll = true);
        x1 = wallLower + wallUpper + mt - kerf + wallSupportWidth + 2 * kc;
        translate([x1, 0, 0])
            mount(knoll = true);
        
        x2 = x1 + kc + 2 * trueBaseRadius + baseRadius + 2 * mt;
        
        translate([x2, baseRadius, 0])
            base();
        
        x3 = x2 + kc + 2 * baseRadius;
        translate([x3, diskRadius, 0])
        disk();
    }
}
else
{
    walls();
    mount();
    translate([0, 0, wallLower])
        base();
    translate([0, 0, wallLower + mt + wallUpper])
        disk();    
}

//wallWithSupport(wallUpper, wallUpper);
