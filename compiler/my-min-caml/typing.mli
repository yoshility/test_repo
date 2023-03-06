exception Error of Syntax.t * Type.t * Type.t
exception Unify of Type.t * Type.t
val extenv : Type.t M.t ref
val f : Syntax.t -> Syntax.t
val unify:Type.t -> Type.t -> unit
val typing : bool ref