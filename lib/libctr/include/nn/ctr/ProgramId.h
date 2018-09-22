/*
ProgramId.h
(c) 2018 Jakcron

This is a wrapper class for the 64bit unique id assigned to programs
*/
#pragma once
#include <nn/ctr/prog_id.h>

namespace nn
{
namespace ctr
{
	class ProgramId
	{
	public:
		ProgramId();
		ProgramId(const ProgramId &other);
		ProgramId(uint64_t prog_id);
		ProgramId(uint16_t device_group, uint16_t category, uint32_t unique_id, uint8_t variation);
	
		void operator=(const ProgramId& other);
		bool operator==(const ProgramId& other) const;
		bool operator!=(const ProgramId& other) const;

		// mutators
		void setInnerValue(uint64_t id);
		uint64_t getInnerValue() const;

		void setDeviceGroup(uint16_t device_group);
		uint16_t getDeviceGroup() const;

		void setCategory(uint16_t category);
		uint16_t getCategory() const;

		void setUniqueId(uint32_t uid);
		uint32_t getUniqueId() const;

		void setVariation(uint8_t variation);
		uint8_t getVariation() const;

	private:
			

		uint64_t mProgramId;
	};
}
}