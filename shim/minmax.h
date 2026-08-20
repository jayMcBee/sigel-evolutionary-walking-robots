// Placeholder for Dynamo/Src/Inc/minmax.h. That header defines min/max as
// MACROS, which poisons every later <algorithm> and Qt header that names
// std::min or std::max. Compile with -DMINMAX_H to suppress it; std::min and
// std::max are already visible through shim/iostream.h's using-directive.
