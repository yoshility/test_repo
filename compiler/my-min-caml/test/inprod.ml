let rec getx v = (let (x, y, z) = v in x) in
let rec gety v = (let (x, y, z) = v in y) in
let rec getz v = (let (x, y, z) = v in z) in
let rec inprod v1 v2 =
  (* getx v1 *. getx v2 +. gety v1 *. gety v2 in *)
getx v1 *. getx v2 +. gety v1 *. gety v2 +. getz v1 *. getz v2 in
print_int (int_of_float (inprod (1., 2., 3.) (4., 5., 6.)))
(* print_int (int_of_float (1.*.2.)) *)
