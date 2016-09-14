#include <unidefs.h>
#include <base_utils/IFail.hxx>
#include <base_utils/Mem.h>
#include <base_utils/ResultCheck.hxx>
#include <Error_Exception.hxx>
#include <tc/emh.h>
#include <tc/emh_errors.h>
#include <tc/tc_startup.h>
#include <iostream>
#include <string>
#include <xstring>

#include "process_error.hxx"

namespace {
/**
 *
 */
std::string decode_error(int code) throw () {
	try {
		char * data = NULL;
		ResultCheck() = EMH_ask_error_text(code, &data);
		if (data) {
			std::string data_back = data;
			MEM_free(data);
			return data_back;
		}
		return "Unable to decode error code in decode_error";
	} catch (...) {
	}
	return "Unhandled exception in decode_error";
}
/**
 *
 */
void do_rethrow(bool rethrow) {
	if (rethrow)
		throw;
}
}

/**
 *
 */
int sisw::process_error(bool log_console, bool log_syslog, bool rethrow) {
	try {
		throw;
	} catch (IFail & erc) {
		EMH_store_error(EMH_severity_error, erc.operator int());
		if (log_console)
			std::cout << decode_error(erc.operator int()) << std::endl;
		if (log_syslog)
			TC_write_syslog("Error: %s\n",
					decode_error(erc.operator int()).c_str());
		if (rethrow)
			throw;
		return erc.operator int();
	} catch (const UGS::Error::Exception & erc) {
		EMH_store_error(EMH_severity_error, erc.askCode());
		if (log_console)
			std::cout << erc.askSyslogMessage() << std::endl;
		if (log_syslog)
			TC_write_syslog("Error: %s\n", erc.askSyslogMessage());
		do_rethrow(rethrow);
		return erc.askCode();
	} catch (const std::exception & erc) {
		EMH_store_error_s1(EMH_severity_error, EMH_GENERIC_ERROR, erc.what());
		if (log_console)
			std::cout << erc.what() << std::endl;
		if (log_syslog)
			TC_write_syslog("Error: %s\n", erc.what());
		do_rethrow(rethrow);
		return EMH_GENERIC_ERROR;
	} catch (...) {
		EMH_store_error_s1(EMH_severity_error, EMH_GENERIC_ERROR,
				"Unhandled exception");
		if (log_console)
			std::cout << "Unhandled exception" << std::endl;
		if (log_syslog)
			TC_write_syslog("Error: %s\n", "Unhandled exception");
		do_rethrow(rethrow);
		return EMH_GENERIC_ERROR;
	}
	do_rethrow(rethrow);
	return EMH_GENERIC_ERROR;
}

