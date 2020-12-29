type os =
    | Windows
    | Mac
    | Linux
    | Unknown

let uname () =
    let ic = Unix.open_process_in "uname" in
    let uname = input_line ic in
    let () = close_in ic in
    uname;;

let get_os =
    match Sys.os_type with
    | "Win32" -> Windows
    | _ -> match uname () with
        | "Darwin" -> Mac
        | "Linux" -> Linux
        | _ -> Unknown

let fzyIncludePath = Sys.getenv "FZY_INCLUDE_PATH"
let fzyLibPath = Sys.getenv "FZY_LIB_PATH"
let c_flags = ["-I"; fzyIncludePath ]

let _ = print_endline (fzyIncludePath)
let _ = print_endline (fzyLibPath)

let ccopt s = ["-ccopt"; s]
let cclib s = ["-cclib"; s]

let libPath = "-L" ^ fzyLibPath

let flags = 
        match get_os with
        | Windows -> []
        @ ccopt(libPath)
        @ cclib("-lfzy")
        @ cclib("-lpthread")
        | _ -> []
        @ ccopt(libPath)
        @ cclib("-lfzy")
        @ cclib("-pthread")
;;

let flags_with_sanitize =
    match get_os with
    | Linux -> flags @ ccopt("-fsanitize=address")
    | _ -> flags
;;

let cxx_flags = c_flags
;;

Configurator.V1.Flags.write_sexp "c_flags.sexp" c_flags;
Configurator.V1.Flags.write_sexp "cxx_flags.sexp" cxx_flags;
Configurator.V1.Flags.write_sexp "flags.sexp" flags;
Configurator.V1.Flags.write_sexp "flags_with_sanitize.sexp" flags_with_sanitize;

