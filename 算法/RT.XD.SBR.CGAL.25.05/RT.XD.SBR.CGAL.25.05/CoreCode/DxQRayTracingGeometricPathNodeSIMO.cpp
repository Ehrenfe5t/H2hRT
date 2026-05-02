#include"DxQRayTracingGeometricPathNodeSIMO.h"

#include"DxQRayTracingGeometricPathNodeToMultiPathNodeInfo.h"
#include"DxQThreadPool.h"
#include"QzQGeometry3DOperate.Distance.h"


namespace RayTracingGeometricPathNodeSIMOStd {


	RayTracingGeometricPathNodeSIMO::RayTracingGeometricPathNodeSIMO(const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna, const std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas)
	{
		this->transmittingAntennaId = transmittingAntenna.transmittingAntennaId;
		this->rayTracingGeometricPathNodeSISOs.clear();
		for (int loop = 0; loop < receiverAntennas.size(); ++loop) {
			RayTracingGeometricPathNodeSISOStd::RayTracingGeometricPathNodeSISO rayTracingGeometricPathNodeSISO;
			rayTracingGeometricPathNodeSISO.receiverAntennaId = receiverAntennas[loop].receiverAntennaId;
			this->rayTracingGeometricPathNodeSISOs.emplace_back(rayTracingGeometricPathNodeSISO);
		}
	}

	RayTracingGeometricPathNodeSIMO::~RayTracingGeometricPathNodeSIMO()
	{
	}

	void RayTracingGeometricPathNodeSIMO::AddPath(int loop_index, const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& path)
	{
		this->rayTracingGeometricPathNodeSISOs[loop_index].Add(path);
	}

	bool SameRayTracingGeometricPathNode(
		double deduplicateRadius,
		const RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode& rayTracingGeometricPathNode1,
		const RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode& rayTracingGeometricPathNode2) {

		double distanceX = rayTracingGeometricPathNode1.location.x - rayTracingGeometricPathNode2.location.x;
		double distanceY = rayTracingGeometricPathNode1.location.y - rayTracingGeometricPathNode2.location.y;
		double distanceZ = rayTracingGeometricPathNode1.location.z - rayTracingGeometricPathNode2.location.z;

		double distance = sqrt(distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ);

		//删除反射和绕射的重合路线，第二删除反射和漫散射的重合路线
		if (distance < 0.001) {
			return true;
		}

		if (rayTracingGeometricPathNode1.type == PropagationTypeStd::PropagationType::DiffuseScattering) {

			return false;

		}
		if (rayTracingGeometricPathNode2.type == PropagationTypeStd::PropagationType::DiffuseScattering) {

			return false;

		}

		if (rayTracingGeometricPathNode1.type != rayTracingGeometricPathNode1.type) {

			return false;

		}

		if (distance < deduplicateRadius) {
			return true;
		}

		return false;
	}

	bool SameRayTracingGeometricPathNodeSISOOnePath(
		double deduplicateRadius,
		const RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath& rayTracingGeometricPathNodeSISOOnePath1,
		const RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath& rayTracingGeometricPathNodeSISOOnePath2) {

		if (rayTracingGeometricPathNodeSISOOnePath2.path.size() < 2) {
			return true;
		}

		if (rayTracingGeometricPathNodeSISOOnePath1.path.size() != rayTracingGeometricPathNodeSISOOnePath2.path.size()) {
			return false;
		}

		for (int i = 0; i < rayTracingGeometricPathNodeSISOOnePath1.path.size(); ++i) {

			if (!SameRayTracingGeometricPathNode(
				deduplicateRadius, 
				rayTracingGeometricPathNodeSISOOnePath1.path[i], rayTracingGeometricPathNodeSISOOnePath2.path[i])) {
				return false;
			}

		}

		return true;

	}

	void DeleteSamePath_SISO_path(double deduplicateRadius,
		std::vector<RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath>& paths) {
		if (paths.size() < 2) {
			return;
		}

		std::sort(paths.begin(), paths.end(), [](
			const RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath& a,
			const RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath& b) {
				return a.length < b.length;
			});

		for (size_t j = paths.size() - 1; j > 0; j--)
		{

			for (size_t i = 0; i < j; ++i) {
				if (SameRayTracingGeometricPathNodeSISOOnePath(deduplicateRadius,
					paths[i], paths[j])) {
					paths.erase(paths.begin() + j);
					break;
				}
			}
		}

		//std::cout <<"...:" << rayTracingGeometricPathNodeSISO.paths.size() << std::endl;
	}

	void DeleteSamePath_SISO(double deduplicateRadius,
		RayTracingGeometricPathNodeSISOStd::RayTracingGeometricPathNodeSISO& rayTracingGeometricPathNodeSISO) {
		if (rayTracingGeometricPathNodeSISO.paths.size() < 2) {
			return;
		}


		for (int loop = 0; loop < rayTracingGeometricPathNodeSISO.paths.size(); ++loop) {

			DeleteSamePath_SISO_path(deduplicateRadius,  rayTracingGeometricPathNodeSISO.paths[loop]);

		}


		//std::cout <<"...:" << rayTracingGeometricPathNodeSISO.paths.size() << std::endl;
	}

	namespace DeleteSamePath_SIMO_MultithreadStd {

		double deduplicateRadius_multithread;
		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO* rayTracingGeometricPathNodeSIMO_multithread;


		typedef struct DefDeleteSamePath_SIMO_Multithread_struct {
			int start;
			int end;

		}DeleteSamePath_SIMO_Multithread_struct;
		void DeleteSamePath_SIMO_Multithread_core(DefDeleteSamePath_SIMO_Multithread_struct& obj) {

			for (int i = obj.start; i < obj.end && i < rayTracingGeometricPathNodeSIMO_multithread->rayTracingGeometricPathNodeSISOs.size(); ++i) {
				DeleteSamePath_SISO(
					deduplicateRadius_multithread,
					rayTracingGeometricPathNodeSIMO_multithread->rayTracingGeometricPathNodeSISOs[i]);
			}
		}

		void DeleteSamePath_SIMO_Multithread(
			double deduplicateRadius,
			RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO) {

			deduplicateRadius_multithread = deduplicateRadius;
			rayTracingGeometricPathNodeSIMO_multithread = &rayTracingGeometricPathNodeSIMO;

			int indexGap = 1;
			std::vector<int> starts;
			for (int i = 0; i < rayTracingGeometricPathNodeSIMO_multithread->rayTracingGeometricPathNodeSISOs.size(); i = i + indexGap) {
				starts.emplace_back(i);
			}

			ThreadPoolStd::ThreadPool threadPool;


			for (int i = 0; i < starts.size(); ++i) {
				DefDeleteSamePath_SIMO_Multithread_struct defDeleteSamePath_SIMO_Multithread_struct;
				defDeleteSamePath_SIMO_Multithread_struct.start = starts[i];
				defDeleteSamePath_SIMO_Multithread_struct.end = starts[i] + indexGap;

				threadPool.submit(DeleteSamePath_SIMO_Multithread_core, defDeleteSamePath_SIMO_Multithread_struct);

			}

			threadPool.join();

		}
	}



	void DeleteSamePath_SIMO(
		bool switchOfMultithread,
		double deduplicateRadius,
		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO) {


		if (switchOfMultithread) {
			DeleteSamePath_SIMO_MultithreadStd::DeleteSamePath_SIMO_Multithread(deduplicateRadius,rayTracingGeometricPathNodeSIMO);
		}
		else {
			for (int i = 0; i < rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs.size(); ++i) {
				DeleteSamePath_SISO(deduplicateRadius, rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs[i]);
			}
		}

	}

	bool RayTracingGeometricPathNodeToMultiPathNodeInfo_SISO_OnePath(
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		const RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath& rayTracingGeometricPathNodeSISOOnePath,
		AntennaSISOOnePathStd::AntennaSISOOnePath& antennaSISOOnePath) {

		if (RayTracingGeometricPathNodeToMultiPathNodeInfoStd::RayTracingGeometricPathNodeToMultiPathNodeInfo_Path(
			rayTracingGeometricPathNodeSISOOnePath.path,
			&scenarioDataInformation,
			antennaSISOOnePath.path)) {
			antennaSISOOnePath.pathLength = rayTracingGeometricPathNodeSISOOnePath.length;

			return true;
		}

		return false;
	}

	void RayTracingGeometricPathNodeToMultiPathNodeInfo_SISO(
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		RayTracingGeometricPathNodeSISOStd::RayTracingGeometricPathNodeSISO& rayTracingGeometricPathNodeSISO,
		AntennaSISOPathStd::AntennaSISOPath& antennaSISOPath) {

		for (int i = 0; i < rayTracingGeometricPathNodeSISO.paths.size(); ++i) {
			if (rayTracingGeometricPathNodeSISO.paths[i].size() > 0) {
				for (int j = (int)rayTracingGeometricPathNodeSISO.paths[i].size() - 1; j >= 0; j--) {
					AntennaSISOOnePathStd::AntennaSISOOnePath antennaSISOOnePath;
					if (RayTracingGeometricPathNodeToMultiPathNodeInfo_SISO_OnePath(scenarioDataInformation, rayTracingGeometricPathNodeSISO.paths[i][j], antennaSISOOnePath)) {
						antennaSISOPath.paths.emplace_back(antennaSISOOnePath);
					}
				}
			}
		}
		rayTracingGeometricPathNodeSISO.paths.clear();
	}


	namespace RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_MultithreadStd {

		ScenarioDataInformationStd::ScenarioDataInformation* scenarioDataInformation_multithread;
		AntennaSIMOPathStd::AntennaSIMOPath* antennaSIMOPath_multithread;

		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO* rayTracingGeometricPathNodeSIMO_multithread;


		typedef struct DefRayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct {
			int start;
			int end;

		}RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct;
		void RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_core(RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct& obj) {

			for (int i = obj.start; i < obj.end && i < rayTracingGeometricPathNodeSIMO_multithread->rayTracingGeometricPathNodeSISOs.size(); ++i) {

				RayTracingGeometricPathNodeToMultiPathNodeInfo_SISO(
					*scenarioDataInformation_multithread,
					rayTracingGeometricPathNodeSIMO_multithread->rayTracingGeometricPathNodeSISOs[i],
					antennaSIMOPath_multithread->paths[i]);
			}
		}

		void RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread(
			ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
			RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO,
			AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath) {

			scenarioDataInformation_multithread = &scenarioDataInformation;
			antennaSIMOPath_multithread = &antennaSIMOPath;
			rayTracingGeometricPathNodeSIMO_multithread = &rayTracingGeometricPathNodeSIMO;

			int indexGap = 1;
			std::vector<int> starts;
			for (int i = 0; i < rayTracingGeometricPathNodeSIMO_multithread->rayTracingGeometricPathNodeSISOs.size(); i = i + indexGap) {
				starts.emplace_back(i);
			}

			ThreadPoolStd::ThreadPool threadPool;


			antennaSIMOPath_multithread->paths.resize(rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs.size());
			for (int i = 0; i < starts.size(); ++i) {
				RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct rayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct;
				rayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct.start = starts[i];
				rayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct.end = starts[i] + indexGap;

				threadPool.submit(RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_core, rayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread_struct);

			}

			threadPool.join();

		}
	}

	void RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO(
		bool switchOfMultithread,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO,
		AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath) {

		if (switchOfMultithread) {
			RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_MultithreadStd::
				RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO_Multithread(scenarioDataInformation, rayTracingGeometricPathNodeSIMO, antennaSIMOPath);
		}
		else {
			antennaSIMOPath.paths.resize(rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs.size());
			for (int i = 0; i < rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs.size(); ++i) {

				RayTracingGeometricPathNodeToMultiPathNodeInfo_SISO(
					scenarioDataInformation,
					rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs[i],
					antennaSIMOPath.paths[i]);
			}
		}


	}

}
