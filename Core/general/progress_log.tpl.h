#ifndef progress_logTplH
#define progress_logTplH

/******************************************************************

progress logger unit.

******************************************************************/


template <class C>
void Progress::Log::Execute(const String&name, Visualizer&visualizer, void (*func_)(C&), C& param)
{
    Begin(name,visualizer);
    func_(param);
}










#endif

