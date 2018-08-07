#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nn/hac/nacp.h>

namespace nn
{
namespace hac
{
	class ApplicationControlPropertyBinary :
		public fnd::ISerialisable
	{
	public:
		struct sTitle
		{
			nacp::Language language;
			std::string name;
			std::string publisher;

			void operator=(const sTitle& other)
			{
				language = other.language;
				name = other.name;
				publisher = other.publisher;
			}

			bool operator==(const sTitle& other) const
			{
				return (language == other.language) \
					&& (name == other.name) \
					&& (publisher == other.publisher);
			}

			bool operator!=(const sTitle& other) const
			{
				return !operator==(other);
			}
		};

		struct sRating
		{
			nacp::Organisation organisation;
			int8_t age;

			void operator=(const sRating& other)
			{
				organisation = other.organisation;
				age = other.age;
			}

			bool operator==(const sRating& other) const
			{
				return (organisation == other.organisation) \
					&& (age == other.age);
			}

			bool operator!=(const sRating& other) const
			{
				return !operator==(other);
			}
		};

		struct sStorageSize
		{
			int64_t size;
			int64_t journal_size;

			void operator=(const sStorageSize& other)
			{
				size = other.size;
				journal_size = other.journal_size;
			}

			bool operator==(const sStorageSize& other) const
			{
				return (size == other.size) \
					&& (journal_size == other.journal_size);
			}

			bool operator!=(const sStorageSize& other) const
			{
				return !operator==(other);
			}
		};

		ApplicationControlPropertyBinary();
		ApplicationControlPropertyBinary(const ApplicationControlPropertyBinary& other);

		void operator=(const ApplicationControlPropertyBinary& other);
		bool operator==(const ApplicationControlPropertyBinary& other) const;
		bool operator!=(const ApplicationControlPropertyBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const fnd::List<sTitle>& getTitle() const;
		void setTitle(const fnd::List<sTitle>& title);

		const std::string& getIsbn() const;
		void setIsbn(const std::string& isbn);

		nacp::StartupUserAccount getStartupUserAccount() const;
		void setStartupUserAccount(nacp::StartupUserAccount var);

		nacp::TouchScreenUsageMode getTouchScreenUsageMode() const;
		void setTouchScreenUsageMode(nacp::TouchScreenUsageMode var);

		nacp::AocRegistrationType getAocRegistrationType() const;
		void setAocRegistrationType(nacp::AocRegistrationType var);

		nacp::AttributeFlag getAttributeFlag() const;
		void setAttributeFlag(nacp::AttributeFlag var);

		const fnd::List<nacp::Language>& getSupportedLanguages() const;
		void setSupportedLanguages(const fnd::List<nacp::Language>& var);

		nacp::ParentalControlFlag getParentalControlFlag() const;
		void setParentalControlFlag(nacp::ParentalControlFlag var);

		nacp::ScreenshotMode getScreenshotMode() const;
		void setScreenshotMode(nacp::ScreenshotMode var);

		nacp::VideoCaptureMode getVideoCaptureMode() const;
		void setVideoCaptureMode(nacp::VideoCaptureMode var);

		nacp::DataLossConfirmation getDataLossConfirmation() const;
		void setDataLossConfirmation(nacp::DataLossConfirmation var);

		nacp::PlayLogPolicy getPlayLogPolicy() const;
		void setPlayLogPolicy(nacp::PlayLogPolicy var);

		uint64_t getPresenceGroupId() const;
		void setPresenceGroupId(uint64_t var);

		const fnd::List<sRating>& getRatingAge() const;
		void setRatingAge(const fnd::List<sRating>& var);

		const std::string& getDisplayVersion() const;
		void setDisplayVersion(const std::string& var);

		uint64_t getAocBaseId() const;
		void setAocBaseId(uint64_t var);

		uint64_t getSaveDatawOwnerId() const;
		void setSaveDatawOwnerId(uint64_t var);

		const sStorageSize& getUserAccountSaveDataSize() const;
		void setUserAccountSaveDataSize(const sStorageSize& var);

		const sStorageSize& getDeviceSaveDataSize() const;
		void setDeviceSaveDataSize(const sStorageSize& var);

		int64_t getBcatDeliveryCacheStorageSize() const;
		void setBcatDeliveryCacheStorageSize(int64_t var);

		const std::string& getApplicationErrorCodeCategory() const;
		void setApplicationErrorCodeCategory(const std::string& var);

		const fnd::List<uint64_t>& getLocalCommunicationId() const;
		void setLocalCommunicationId(const fnd::List<uint64_t>& var);

		nacp::LogoType getLogoType() const;
		void setLogoType(nacp::LogoType var);

		nacp::LogoHandling getLogoHandling() const;
		void setLogoHandling(nacp::LogoHandling var);

		nacp::RuntimeAocInstallMode getRuntimeAocInstallMode() const;
		void setRuntimeAocInstallMode(nacp::RuntimeAocInstallMode var);

		nacp::CrashReportMode getCrashReportMode() const;
		void setCrashReportMode(nacp::CrashReportMode var);

		nacp::Hdcp getHdcp() const;
		void setHdcp(nacp::Hdcp var);

		uint64_t getSeedForPsuedoDeviceId() const;
		void setSeedForPsuedoDeviceId(uint64_t var);

		const std::string& getBcatPassphase() const;
		void setBcatPassphase(const std::string& var);

		const sStorageSize& getUserAccountSaveDataMax() const;
		void setUserAccountSaveDataMax(const sStorageSize& var);

		const sStorageSize& getDeviceSaveDataMax() const;
		void setDeviceSaveDataMax(const sStorageSize& var);

		int64_t getTemporaryStorageSize() const;
		void setTemporaryStorageSize(int64_t var);

		const sStorageSize& getCacheStorageSize() const;
		void setCacheStorageSize(const sStorageSize& var);

		int64_t getCacheStorageDataAndJournalSizeMax() const;
		void setCacheStorageDataAndJournalSizeMax(int64_t var);

		uint16_t getCacheStorageIndexMax() const;
		void setCacheStorageIndexMax(uint16_t var);

		const fnd::List<uint64_t>& getPlayLogQueryableApplicationId() const;
		void setPlayLogQueryableApplicationId(const fnd::List<uint64_t>& var);

		nacp::PlayLogQueryCapability getPlayLogQueryCapability() const;
		void setPlayLogQueryCapability(nacp::PlayLogQueryCapability var);

		nacp::RepairFlag getRepairFlag() const;
		void setRepairFlag(nacp::RepairFlag var);

		byte_t getProgramIndex() const;
		void setProgramIndex(byte_t var);


	private:
		const std::string kModuleName = "APPLICATION_CONTROL_PROPERTY";

		// raw data
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::List<sTitle> mTitle;
		std::string mIsbn;
		nacp::StartupUserAccount mStartupUserAccount;
		nacp::TouchScreenUsageMode mTouchScreenUsageMode;
		nacp::AocRegistrationType mAocRegistrationType;
		nacp::AttributeFlag mAttributeFlag;
		fnd::List<nn::hac::nacp::Language> mSupportedLanguages;
		nacp::ParentalControlFlag mParentalControlFlag;
		nacp::ScreenshotMode mScreenshotMode;
		nacp::VideoCaptureMode mVideoCaptureMode;
		nacp::DataLossConfirmation mDataLossConfirmation;
		nacp::PlayLogPolicy mPlayLogPolicy;
		uint64_t mPresenceGroupId;
		fnd::List<sRating> mRatingAge;
		std::string mDisplayVersion;
		uint64_t mAocBaseId;
		uint64_t mSaveDatawOwnerId;
		sStorageSize mUserAccountSaveDataSize;
		sStorageSize mDeviceSaveDataSize;
		int64_t mBcatDeliveryCacheStorageSize;
		std::string mApplicationErrorCodeCategory;
		fnd::List<uint64_t> mLocalCommunicationId;
		nacp::LogoType mLogoType;
		nacp::LogoHandling mLogoHandling;
		nacp::RuntimeAocInstallMode mRuntimeAocInstallMode;
		nacp::CrashReportMode mCrashReportMode;
		nacp::Hdcp mHdcp;
		uint64_t mSeedForPsuedoDeviceId;
		std::string mBcatPassphase;
		sStorageSize mUserAccountSaveDataMax;
		sStorageSize mDeviceSaveDataMax;
		int64_t mTemporaryStorageSize;
		sStorageSize mCacheStorageSize;
		int64_t mCacheStorageDataAndJournalSizeMax;
		uint16_t mCacheStorageIndexMax;
		fnd::List<uint64_t> mPlayLogQueryableApplicationId;
		nacp::PlayLogQueryCapability mPlayLogQueryCapability;
		nacp::RepairFlag mRepairFlag;
		byte_t mProgramIndex;
	};
}
}