#include "configdata.h"
#include "value.h"
#include <sstream>

std::shared_ptr<sqf::value> sqf::configdata::parent_unsafe(void)
{
	std::weak_ptr<configdata> lparent = mlogicparent;
	while (!lparent.expired())
	{
		auto lockparent = lparent.lock();
		lparent = lockparent->mlogicparent;
		auto res = navigate_unsafe(mname);
		if (res.get())
		{
			return res;
		}
	}
	return std::shared_ptr<sqf::value>();
}

std::shared_ptr<sqf::value> sqf::configdata::navigate_unsafe(std::wstring nextnode)
{
	for each (auto it in innervector())
	{
		if (it->dtype() != type::CONFIG)
			continue;
		auto cd = it->data<sqf::configdata>();
		if (wstr_cmpi(cd->mname.c_str(), -1, nextnode.c_str(), -1) == 0)
			return it;
	}
	return std::shared_ptr<sqf::value>();
}

std::shared_ptr<sqf::value> sqf::configdata::navigate_full_unsafe(std::wstring nextnode)
{
	auto it = navigate_unsafe(nextnode);
	if (it.get())
	{
		return it;
	}
	else
	{
		std::shared_ptr<sqf::value> p;
		while ((p = parent_unsafe()).get())
		{
			auto it = navigate_full_unsafe(nextnode);
			if (it.get())
				return it;
		}
		return std::shared_ptr<sqf::value>();
	}
}

std::wstring sqf::configdata::tosqf(void) const
{
	std::wstringstream sstream;
	for (size_t i = logicalparentcount(); i != ~0; i--)
	{
		auto node = std::shared_ptr<configdata>((configdata*)this, [](configdata*) {});;
		for (size_t j = 0; j < i; j++)
		{
			node = node->mlogicparent.lock();
		}
		sstream << node->mname;
		if (i != 0)
		{
			sstream << L'/';
		}
	}
	return sstream.str();
}

std::shared_ptr<sqf::value> sqf::configdata::logicparent(void)
{
	return mlogicparent.expired() ? configNull() : std::make_shared<sqf::value>(mlogicparent.lock(), type::CONFIG);
}

void sqf::configdata::mergeinto(std::shared_ptr<configdata> cd)
{
	for each (auto val in innervector())
	{
		if (val->dtype() != sqf::type::CONFIG)
			continue;
		auto subcd = val->data<configdata>();
		auto othercd = cd->navigate_unsafe(subcd->mname);
		if (othercd.get())
		{
			subcd->mergeinto(othercd->data<configdata>());
		}
		else
		{
			cd->push_back(val);
		}
	}
}

std::shared_ptr<sqf::value> sqf::configdata::configFile(void)
{
	static std::shared_ptr<sqf::configdata> cdata = std::make_shared<sqf::configdata>();
	return std::make_shared<sqf::value>(cdata, sqf::type::CONFIG);
}

std::shared_ptr<sqf::value> sqf::configdata::configNull(void)
{
	static std::shared_ptr<sqf::configdata> cdata = std::make_shared<sqf::configdata>(L"");
	return std::make_shared<sqf::value>(cdata, sqf::type::CONFIG);
}
