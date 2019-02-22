#include "MxVkModel.h"

namespace Mix {
    namespace Vk {
        void ModelImporter::processNode(fbxsdk::FbxNode * node, Model & model) {
            if (!(node->GetNodeAttribute()))
                return;

            switch (node->GetNodeAttribute()->GetAttributeType()) {
            case fbxsdk::FbxNodeAttribute::eMesh:
                processMesh(node->GetMesh(), model);
                break;
            default:
                break;
            }

            for (int i = 0; i < node->GetChildCount(); ++i) {
                processNode(node->GetChild(i), model);
            }
        }

        void ModelImporter::processMesh(fbxsdk::FbxMesh * mesh, Model & model) {
            if (!mesh)
                return;

            Mesh tempMesh;
            tempMesh.mTriangleCount = mesh->GetPolygonCount();
            tempMesh.mVertices.reserve(tempMesh.mTriangleCount * 3);

            fbxsdk::FbxVector4* controlPoints = mesh->GetControlPoints();
            fbxsdk::FbxGeometryElementUV* uv = mesh->GetElementUV();
            fbxsdk::FbxGeometryElementNormal* normal = mesh->GetElementNormal();

            Vertex vertex;
            int vertexCount = 0;
            int controlPointIndex = 0;
            for (int i = 0; i < tempMesh.mTriangleCount; ++i) {
                for (int j = 0; j < 3; ++j) {
                    controlPointIndex = mesh->GetPolygonVertex(i, j);
                    //get position
                    readPosition(controlPoints, controlPointIndex, vertex);
                    readUV(mesh, uv, controlPointIndex, i, j, vertex);
                    readNormal(mesh, normal, vertexCount, vertex);
                    tempMesh.mVertices.push_back(vertex);
                    ++vertexCount;
                }
            }
            model.mMeshes.push_back(tempMesh);
        }

        void ModelImporter::readPosition(fbxsdk::FbxVector4 * ctrPoints, int ctrPointIndex, Vertex & v) {
            v.pos.x = ctrPoints[ctrPointIndex][0];
            v.pos.y = ctrPoints[ctrPointIndex][1];
            v.pos.z = ctrPoints[ctrPointIndex][2];
        }

        void ModelImporter::readUV(fbxsdk::FbxMesh* mesh, fbxsdk::FbxGeometryElementUV * uv, int ctrPointIndex, int polygonIndex, int vertexIndex, Vertex & v) {
            //get uv
            switch (uv->GetMappingMode()) {
            case fbxsdk::FbxGeometryElement::eByControlPoint:
                switch (uv->GetReferenceMode()) {
                case fbxsdk::FbxGeometryElement::eDirect:
                    v.texCoord.x = uv->GetDirectArray().GetAt(ctrPointIndex)[0];
                    v.texCoord.y = uv->GetDirectArray().GetAt(ctrPointIndex)[1];
                    break;
                case fbxsdk::FbxGeometryElement::eIndexToDirect:
                    int index = uv->GetIndexArray().GetAt(ctrPointIndex);
                    v.texCoord.x = uv->GetDirectArray().GetAt(index)[0];
                    v.texCoord.y = uv->GetDirectArray().GetAt(index)[1];
                    break;
                }
                break;

            case fbxsdk::FbxGeometryElement::eByPolygonVertex:
                int uvIndex = mesh->GetTextureUVIndex(polygonIndex, vertexIndex);

                switch (uv->GetReferenceMode()) {
                case fbxsdk::FbxGeometryElement::eDirect:
                case fbxsdk::FbxGeometryElement::eIndexToDirect:
                    v.texCoord.x = uv->GetDirectArray().GetAt(uvIndex)[0];
                    v.texCoord.y = uv->GetDirectArray().GetAt(uvIndex)[1];
                    break;
                }
                break;
            }//getuv
        }

        void ModelImporter::readNormal(fbxsdk::FbxMesh * mesh, fbxsdk::FbxGeometryElementNormal * normal, int vertexCount, Vertex & v) {
            //get normal
            switch (normal->GetMappingMode()) {
            case fbxsdk::FbxGeometryElement::eByControlPoint:
                break;
            case fbxsdk::FbxGeometryElement::eByPolygonVertex:
                switch (normal->GetReferenceMode()) {
                case fbxsdk::FbxGeometryElement::eDirect:
                    v.normal.x = normal->GetDirectArray().GetAt(vertexCount)[0];
                    v.normal.y = normal->GetDirectArray().GetAt(vertexCount)[1];
                    v.normal.z = normal->GetDirectArray().GetAt(vertexCount)[2];
                    break;
                case fbxsdk::FbxGeometryElement::eIndexToDirect:
                    int index = normal->GetIndexArray().GetAt(vertexCount);
                    v.normal.x = normal->GetDirectArray().GetAt(index)[0];
                    v.normal.y = normal->GetDirectArray().GetAt(index)[1];
                    v.normal.z = normal->GetDirectArray().GetAt(index)[2];
                    break;
                }
                break;
            }//get normal
        }

        ModelImporter::ModelImporter() :mManager(nullptr), mInitialized(false) {

        }

        ModelImporter::~ModelImporter() {
            destroy();
        }

        bool ModelImporter::initialize() {
            if (!mManager) {
                mManager = fbxsdk::FbxManager::Create();
                if (!mManager) {
                    std::cerr << "Error : Failed to allocate FbxManager!" << std::endl;
                    mInitialized = false;
                    return false;
                } else {
                    mInitialized = true;
                    return true;
                }
            }

            return true;
        }

        Model * ModelImporter::loadModel(const std::string & path) {
            if (!mInitialized) {
                std::cerr << "Error : ModelImporter been used without initialization!" << std::endl;
                return nullptr;
            }

            fbxsdk::FbxIOSettings* iosettings = fbxsdk::FbxIOSettings::Create(mManager, IOSROOT);
            if (!iosettings) {
                std::cerr << "Error : Failed to allocate FbxIOSettiongs!" << std::endl;
                return nullptr;
            }

            mManager->SetIOSettings(iosettings);

            fbxsdk::FbxScene* scene = fbxsdk::FbxScene::Create(mManager, "MyScene");
            if (!scene) {
                std::cerr << "Error : Failed to allocate FbxScene!" << std::endl;
                return nullptr;
            }

            fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(mManager, "");
            if (!importer) {
                std::cerr << "Error : Failed to allocate FbxImporter!" << std::endl;
                return nullptr;
            }

            //load moedl
            if (!importer->Initialize(path.c_str(), -1, mManager->GetIOSettings())) {
                std::cerr << "Warning : FbxImporter :" << std::endl
                    << importer->GetStatus().GetErrorString() << std::endl;
                return nullptr;
            }

#ifdef MX_DEBUG
            std::cout << "File name : " << importer->GetFileName() << std::endl;
            int major, minor, revision;
            importer->GetFileVersion(major, minor, revision);
            std::cout << "File version : " << major << "." << minor << "." << revision << std::endl;
#endif

            importer->Import(scene);
            importer->Destroy();

            Model* model = new Model;
            if (!model) {
                std::cerr << "Error : Failed to allocate Model!" << std::endl;
                return nullptr;
            }

            fbxsdk::FbxNode* root = scene->GetRootNode();
            for (int i = 0; i < root->GetChildCount(); ++i) {
                processNode(root->GetChild(i), *model);
            }
            return model;
        }

        void ModelImporter::destroy() {
            if (mManager) {
                mManager->Destroy();
                mManager = nullptr;
            }
        }

        Mesh::~Mesh() {
            MX_FREE_OBJECT(mVertexBuffer);
        }

        uint32_t Mesh::getTriangleCount() const {
            return mTriangleCount;
        }

        uint32_t Mesh::getVertexCount() const {
            return mTriangleCount * 3;
        }

        const Mesh::vertexType * Mesh::getBuffer() const {
            return mVertices.data();
        }

        Mesh::vertexType * Mesh::getBuffer() {
            return mVertices.data();
        }

        const Buffer * Mesh::getVertexBuffer() const {
            return mVertexBuffer;
        }

        Buffer * Mesh::getVertexBuffer() {
            return mVertexBuffer;
        }

        void Mesh::createVertexBuffer(const Manager * manager, CommandPool* commandPool) {
            VkDeviceSize bufferSize = sizeof(decltype(mVertices)::value_type)*mVertices.size();

            mVertexBuffer = Buffer::createBuffer(manager,
                                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                 bufferSize);

            Buffer::copyToDeviceBuffer(manager, commandPool, mVertexBuffer, mVertices.data());
        }

        void Mesh::draw(const VkCommandBuffer commandBuffer) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer->buffer, &offset);
            vkCmdDraw(commandBuffer, mTriangleCount * 3, 1, 0, 0);
        }

        uint32_t Model::getMeshCount() const {
            return mMeshes.size();
        }

        const Mesh * Model::getMeshes() const {
            return mMeshes.data();
        }

        Mesh * Model::getMeshes() {
            return mMeshes.data();
        }

        void Model::createVertexBuffer(const Manager * manager, CommandPool * commandPool) {
            for (auto& mesh : mMeshes)
                mesh.createVertexBuffer(manager, commandPool);
        }
        void Model::draw(const VkCommandBuffer commandBuffer) {
            std::vector<VkBuffer> vertexBuffers;
            vertexBuffers.reserve(mMeshes.size());

            for (const auto& mesh : mMeshes)
                vertexBuffers.push_back(mesh.getVertexBuffer()->buffer);

            std::vector<VkDeviceSize> offsets(vertexBuffers.size(), 0);

            vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
            for (auto& mesh : mMeshes)
                vkCmdDraw(commandBuffer, mesh.getVertexCount(), 1, 0, 0);
        }
    }
}
