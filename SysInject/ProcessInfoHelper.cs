using System;
using System.Diagnostics;

namespace SysInject
{
    public static class ProcessInfoHelper
    {
        // --------------------
        // Memory Usage
        // --------------------
        public static string ToMB(long bytes)
        {
            return (bytes / (1024.0 * 1024.0)).ToString("F2") + " MB";
        }

        public static string GetWorkingSet(int pid) => ToMB(Process.GetProcessById(pid).WorkingSet64);
        public static string GetPrivateBytes(int pid) => ToMB(Process.GetProcessById(pid).PrivateMemorySize64);
        public static string GetVirtualMemory(int pid) => ToMB(Process.GetProcessById(pid).VirtualMemorySize64);
        public static string GetPagedMemory(int pid) => ToMB(Process.GetProcessById(pid).PagedMemorySize64);
        public static string GetNonpagedMemory(int pid) => ToMB(Process.GetProcessById(pid).NonpagedSystemMemorySize64);
    }
}
