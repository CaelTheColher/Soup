<?php
$max_args = 20;

function call_case(int $args): string
{
	$str = "case ".$args.": return reinterpret_cast<uintptr_t(cc*)(";
	for ($i = 0; $i != $args; ++$i)
	{
		if($i != 0)
		{
			$str .= ", ";
		}
		$str .= "uintptr_t";
	}
	$str .= ")>(func)(";
	for ($i = 0; $i != $args; ++$i)
	{
		if($i != 0)
		{
			$str .= ", ";
		}
		$str .= "args.at(".$i.")";
	}
	$str .= ");";
	return $str;
}

$convs = [
	["CDECL", "__cdecl", "cdeclCall"],
	["FASTCALL", "__fastcall", "fastcall"],
	["STDCALL", "__stdcall", "stdcall"],
	["THISCALL", "__thiscall", "thiscall"],
	["VECTORCALL", "__vectorcall", "vectorcall"],
];

$fh = fopen("ffi.cpp", "w");
fwrite($fh, "#include \"ffi.hpp\"\n\n// Generated by scripts/ffi.php\n\n#define DO_FFI_CALL(cc) \\\n");
fwrite($fh, "switch (args.size()) { \\\n");
for ($i = 0; $i != $max_args + 1; ++$i)
{
	fwrite($fh, call_case($i)." \\\n");
}
fwrite($fh, "} \\\n");
fwrite($fh, "throw BadCall();");
fwrite($fh, "\n\nnamespace soup\n{\nuintptr_t ffi::call(CallConv conv, void* func, const std::vector<uintptr_t>& args)\n{\nswitch(conv){\n");
foreach($convs as $conv)
{
	fwrite($fh, "case ".$conv[0].": return ".$conv[2]."(func, args);\n");
}
fwrite($fh, "}\nthrow BadCall();\n}\n");
foreach($convs as $conv)
{
	fwrite($fh, "uintptr_t ffi::".$conv[2]."(void* func, const std::vector<uintptr_t>& args)\n{\nDO_FFI_CALL(".$conv[1].")\n}\n");
}
fwrite($fh, "}\n");
