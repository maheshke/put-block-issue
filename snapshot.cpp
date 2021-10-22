#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>

#include <aws/core/Aws.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/ebs/model/ListSnapshotBlocksRequest.h>
#include <aws/ebs/model/PutSnapshotBlockRequest.h>
#include <aws/ebs/model/ChecksumAlgorithm.h>
#include <aws/ebs/EBSClient.h>

#define KB (1024)

using namespace std;

struct Args
{
    Args() :
        snapshot()
    {
    }

    bool valid(void)
    {
        if (snapshot.empty())
        {
            printf("Invalid arguments.\n"
                    "\tsnapshot: %s\n", snapshot.c_str());

            return false;
        }

        return true;
    }

    static void usage(void)
    {
        printf("Usage:\n"
                "./put-block-issue --snapshot <EBS snapshot ID> [--list]\n");
    }

    string snapshot;
};

class Snapshot
{
    public:
        Snapshot(std::string snapshot);
        ~Snapshot() = default;

        void listBlocks(void);
        void putOneBlock(void);

    private:
        std::string mSnapshot;

        Aws::EBS::EBSClient mClient;
};

/* Snapshot methods */
Snapshot::Snapshot(string snapshot) :
    mSnapshot(snapshot)
{
    Aws::Client::ClientConfiguration config;

    mClient = Aws::EBS::EBSClient(config);
}

void Snapshot::listBlocks(void)
{
    Aws::EBS::Model::ListSnapshotBlocksRequest request;
    request.SetSnapshotId(mSnapshot);
    request.SetMaxResults(100);

    auto outcome = mClient.ListSnapshotBlocks(request);

    if (outcome.IsSuccess())
    {
        cout << "ListSnapshotBlocks for snapshot " << mSnapshot << " is successful" << std::endl;

        return;
    }
    else
    {
        auto err = outcome.GetError();

        cout << "Error: ListSnapshotBlocks: " << mSnapshot <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;

        return;
    }
}

void Snapshot::putOneBlock(void)
{
    auto blockSize = 512 * KB;
    char buffer[blockSize];

    auto sstream = Aws::MakeShared<Aws::StringStream>("PutSnapshotBlockRequest",
            std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    sstream->write(buffer, blockSize);

    auto byte_checksum = Aws::Utils::HashingUtils::CalculateSHA256(*sstream);
    auto checksum = Aws::Utils::HashingUtils::Base64Encode(byte_checksum);

    Aws::EBS::Model::PutSnapshotBlockRequest request;
    request.SetSnapshotId(mSnapshot);
    request.SetBlockIndex(0);
    request.SetDataLength(blockSize);
    request.SetBody(sstream);
    request.SetChecksum(checksum);
    request.SetChecksumAlgorithm(Aws::EBS::Model::ChecksumAlgorithm::SHA256);

    auto outcome = mClient.PutSnapshotBlock(request);

    if (!outcome.IsSuccess())
    {
        auto err = outcome.GetError();

        cout << "Failed to put block. Snapshot " << mSnapshot <<
            ". Exception - " << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;

        return;
    }
}

int main(int argc, char **argv)
{
    static struct option long_options[] =
    {
	{"snapshot",	required_argument,       0, 's'},
	{"list",	no_argument,             0, 'l'},
	{"help",	no_argument,             0, 'h'},
	{0, 0, 0, 0}
    };

    Args args;
    bool list = false;

    while (true)
    {
        int option_index = 0;

        int c = getopt_long(argc, argv, "s:h",
                long_options, &option_index);

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
            case 's':
                args.snapshot.assign(optarg);
                break;
            case 'l':
                list = true;
                break;
            case 'h':
                Args::usage();
                return -1;
            default:
                printf("Invalid argument:%s\n", optarg);
                Args::usage();
                return -1;
        }
    }

    if (!args.valid())
    {
        Args::usage();
        return -1;
    }

    Aws::SDKOptions options;
    Aws::InitAPI(options);

    {
        Snapshot snapshot(args.snapshot);

        if (list)
        {
            snapshot.listBlocks();
        }
        else
        {
            snapshot.putOneBlock();
        }
    }

    Aws::ShutdownAPI(options);

    return 0;
}
