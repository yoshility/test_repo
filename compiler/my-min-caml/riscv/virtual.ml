(* translation into SPARC assembly with infinite number of virtual registers *)

open Asm

let data = ref [] (* 浮動小数点数の定数テーブル (caml2html: virtual_data) *)

let classify xts ini addf addi =
  List.fold_left
    (fun acc (x, t) ->
      match t with
      | Type.Unit -> acc
      | Type.Float -> addf acc x
      | _ -> addi acc x t)
    ini
    xts

let separate xts =
  classify
    xts
    ([], [])
    (fun (int, float) x -> (int, float @ [x]))
    (fun (int, float) x _ -> (int @ [x], float))

let expand xts ini addf addi =
  classify
    xts
    ini
    (fun (offset, acc) x ->
      let offset = align offset in
      (offset + 4, addf x offset acc))(* 単精度に変更 *)
    (fun (offset, acc) x t ->
      (offset + 4, addi x t offset acc))

let rec g  env (e,p)=  (* 式の仮想マシンコード生成 (caml2html: virtual_g) *)
match e with
  | Closure.Unit -> Ans(Nop,p)
  | Closure.Int(i) -> Ans(Set(i),p)
  | Closure.Float(d) ->
      let l =
        try
          (* すでに定数テーブルにあったら再利用 Cf. https://github.com/esumii/min-caml/issues/13 *)
          let (l, _) = List.find (fun (_, d') -> d = d') !data in
          l
        with Not_found ->
          let l = Id.L(Id.genid "l") in
          data := (l, d) :: !data;
          l in
      let x = Id.genid "l" in
      Let((x, Type.Int), (Flabel(l),p), Ans((LdDF(x, 0),p)))
  | Closure.Neg(x) -> Ans((Neg(x),p))
  | Closure.Add(x, y) -> Ans((Add(x, V(y)),p))
  | Closure.Sub(x, y) -> Ans((Sub(x, V(y)),p))
  | Closure.Mul(x, y) -> Ans((Mul(x, V(y)),p))
  | Closure.Div(x, y) -> Ans((Div(x, V(y)),p))
  | Closure.FNeg(x) -> Ans((FNegD(x),p))
  | Closure.FAdd(x, y) -> Ans((FAddD(x, y),p))
  | Closure.FSub(x, y) -> Ans((FSubD(x, y),p))
  | Closure.FMul(x, y) -> Ans((FMulD(x, y),p))
  | Closure.FDiv(x, y) -> Ans((FDivD(x, y),p))
  | Closure.IfEq(x, y, e1, e2) ->
      (match try (M.find x env) with e -> Printf.eprintf"Error Virtual1 %s" x ;raise e with
      | Type.Bool | Type.Int -> Ans((IfEq(x, V(y), g  env e1, g  env e2),p))
      | Type.Float -> Ans((IfFEq(x, y, g  env e1, g  env e2),p))
      | _ -> failwith "equality supported only for bool, int, and float")
  | Closure.IfLE(x, y, e1, e2) ->
      (match try (M.find x env) with e -> Printf.eprintf"Error Virtual2 %s" x ;raise e with
      | Type.Bool | Type.Int -> Ans((IfLE(x, V(y), g  env e1, g  env e2),p))
      | Type.Float -> Ans((IfFLE(x, y, g  env e1, g  env e2),p))
      | _ -> failwith "inequality supported only for bool, int, and float")
  | Closure.Let((x, t1), e1, e2) ->
      let e1' = g  env e1 in
      let e2' = g  (M.add x t1 env) e2 in
      concat e1' (x, t1) e2'
  | Closure.Var(x) ->
      (match try (M.find x env) with e -> Printf.eprintf"Error Virtual3 %s" x ;raise e with
      | Type.Unit -> Ans((Nop,p))
      | Type.Float -> Ans((FMovD(x),p))
      (* | Type.Fun(_) -> (try (let address = getaddress x in  Ans(Mov(x), p)) with |Not_found -> Ans((Mov(x),p))) *)
      (* | Type.Fun(_) -> Printf.printf "Address of Fun %s\n" x;Ans((Address( x), p)) *)
      (* | Type.Fun(_) -> Printf.printf "Address of Fun %s\n" x;Ans((Mov(reg_cl), p))この実装は間違っているけどraytracerなら大丈夫か？ *)

      | _ -> Ans((Mov(x),p)))
  | Closure.MakeCls((x, t), { Closure.entry = l; Closure.actual_fv = ys }, e2) -> (* クロージャの生成 (caml2html: virtual_makecls) *)
      (* Closureのアドレスをセットしてから、自由変数の値をストア *)
      let e2' = g  (M.add x t env) e2 in
      if effect  e2' then (effect_cls:= (x::!effect_cls)) else ();
      let offset, store_fv =
        expand
          (List.map (fun y -> (y, M.find y env)) ys)
          (4, e2')
          (fun y offset store_fv -> seq((StDF(y, x, offset),p), store_fv))
          (fun y _ offset store_fv -> seq(((St(y, x, offset),p)), store_fv)) in
          let offset = align offset in
      Let((x, t), (Mov(reg_hp),p),(*ここだ*)
          Let((reg_hp, Type.Int), (Add(reg_hp, C(offset)),p),
              let z = Id.genid "l" in
              Let((z, Type.Int), (SetL(l), p),
                  seq((St(z, x, 0),p),
                      store_fv))))
  | Closure.AppCls(x, ys) ->
      let (int, float) = separate (List.map (fun y -> (y, try (M.find y env) with e -> Printf.eprintf"Error Virtual10 %s" y ;raise e)) ys) in
      Ans((CallCls(x, int, float),p))
  | Closure.AppDir(Id.L(x), ys) ->
      let (int, float) = separate (List.map (fun y -> (y, try (M.find y env) with e -> Printf.eprintf"Error Virtual11 %s %s"x  y ;raise e)) ys) in
      if x = "min_caml_float_of_int" then Ans(FofI(List.hd(int)),p)
      else if x = "min_caml_int_of_float" then Ans(IofF(List.hd(float)),p)
      else if x = "min_caml_fabs" then Ans(Fabs(List.hd(float)),p)
      else if x = "min_caml_fneg" then Ans(Fneg(List.hd(float)),p)
      else if x = "min_caml_fless" then Ans(Fless(List.hd(float), List.nth float 1),p)
      else if x = "min_caml_fispos" then Ans(Fispos(List.hd(float)),p)
      else if x = "min_caml_fisneg" then Ans(Fisneg(List.hd(float)),p)
      else if x = "min_caml_fiszero" then Ans(Fiszero(List.hd(float)),p)
      else if x = "min_caml_floor" then Ans(Floor(List.hd(float)),p)
      else if x = "min_caml_fsqr" then Ans(Fsqr(List.hd(float)),p)
      else if x = "min_caml_sqrt" then Ans(Fsqrt(List.hd(float)),p)
      else if x = "min_caml_fhalf" then Ans(Fhalf(List.hd(float)),p)
      else if x = "min_caml_read_int" then Ans(RInt,p)
      else if x = "min_caml_read_float" then Ans(RFloat,p)
      else if x = "min_caml_print_int" then Ans(PInt(List.hd(int)),p)
      else if x = "min_caml_print_char" then Ans(Pchar(List.hd(int)),p)
      else if x = "min_caml_print_newline" then Ans(Pline,p)
      else if x = "min_caml_create_array" then Ans(CArray(V(List.hd(int)), List.nth int 1),p)
      else if x = "min_caml_create_float_array" then Ans(CFArray(V(List.hd(int)), List.hd(float)),p)
      else Ans((CallDir(Id.L(x), int, float),p))
  | Closure.Tuple(xs) -> (* 組の生成 (caml2html: virtual_tuple) *)
      let y = Id.genid "t" in
      let (offset, store) =
        expand
          (List.map (fun x -> (x, try (M.find x env) with e -> Printf.eprintf"Error Virtual4 %s" x ;raise e )) xs)
          (0, Ans((Mov(y),p)))
          (fun x offset store -> seq((StDF(x, y, offset),p), store))
          (fun x _ offset store -> seq((St(x, y, offset),p), store)) in
          let offset = align offset in
      Let(
        (y, Type.Tuple(List.map (fun x -> try (M.find x env) with e -> Printf.eprintf"Error Virtual5 %s" x ;raise e) xs)),
       (Mov(reg_hp),p),
        Let(
          (reg_hp, Type.Int), 
          (Add(reg_hp, C(offset)),p),
          store)
          )
  | Closure.LetTuple(xts, y, e2) ->
      let s = Closure.fv e2 in
      let (offset, load) =
        expand
          xts
          (0, g  (M.add_list xts env) e2)
          (fun x offset load ->
            if not (S.mem x s) then load else (* [XX] a little ad hoc optimization *)
            fletd(x, (LdDF(y, offset),p), load))
          (fun x t offset load ->
            if not (S.mem x s) then load else (* [XX] a little ad hoc optimization *)
            Let((x, t), (Ld(y, offset),p), load)) in
      load
  | Closure.Get(x, y) -> (* 配列の読み出し (caml2html: virtual_get) *)
      let offset = Id.genid "o" in
      let offset2 = Id.genid "o" in
      (match try (M.find x env) with e -> Printf.eprintf"Error Virtual6 %s" x ;raise e with
      | Type.Array(Type.Unit) -> Ans((Nop,p))
      | Type.Array(Type.Float) ->
          Let((offset, Type.Int), (SLL(y, C(2)),p),
          Let((offset2, Type.Int), (Add(x, V(offset)),p),
              Ans((LdDF(offset2, 0),p))))(**)
      | Type.Array(_) ->
        Let((offset, Type.Int), (SLL(y, C(2)),p),
        Let((offset2, Type.Int), (Add(x, V(offset)),p),
            Ans((Ld(offset2, 0),p))))(**)
      | _ -> assert false)
  | Closure.Put(x, y, z) ->
      let offset = Id.genid "o" in
      let offset2 = Id.genid "o" in
      (match try (M.find x env) with e -> Printf.eprintf"Error Virtual7 %s" x ;raise e with
      | Type.Array(Type.Unit) -> Ans((Nop,p))
      | Type.Array(Type.Float) ->
        Let((offset, Type.Int), (SLL(y, C(2)),p),
        Let((offset2, Type.Int), (Add(x, V(offset)),p),
            Ans((StDF(z, offset2, 0),p))))
      | Type.Array(_) ->
          Let((offset, Type.Int), (SLL(y, C(2)),p),
          Let((offset2, Type.Int), (Add(x, V(offset)),p),
              Ans((St(z, offset2, 0),p))))
      | _ -> assert false)
  | Closure.ExtArray(Id.L(x)) -> 
    Printf.printf "%s Extarray!!\n" x ;Ans((SetL(Id.L("min_caml_" ^ x)),p))
    
(* 関数の仮想マシンコード生成 (caml2html: virtual_h) *)
let h  { Closure.name = (Id.L(x), t); Closure.args = yts; Closure.formal_fv = zts; Closure.body = e } =
  let p = snd e in
  let (int, float) = separate yts in
  let (offset, load) =
    expand
      zts
      (4, g  (M.add x t (M.add_list yts (M.add_list zts M.empty))) e)
      (fun z offset load -> fletd(z, (LdDF(reg_cl, offset),p), load))
      (fun z t offset load -> Let((z, t), (Ld(reg_cl, offset),p), load)) in
  match t with
  | Type.Fun(_, t2) ->
      { name = Id.L(x); args = int; fargs = float; body = load; ret = t2 }
  | _ -> assert false

(* プログラム全体の仮想マシンコード生成 (caml2html: virtual_f) *)
let f (Closure.Prog(fundefs, e)) =
  data := [(Id.L("l.001"),0.5)];
  let fundefs = List.map (h ) fundefs in
  let e = g  M.empty e in Printf.eprintf "Virtual finished\n";
  Prog(!data, fundefs, e)
