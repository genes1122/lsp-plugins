/*
 * KVTStorage.h
 *
 *  Created on: 30 мая 2019 г.
 *      Author: sadko
 */

#ifndef CORE_KVTSTORAGE_H_
#define CORE_KVTSTORAGE_H_

#include <core/types.h>
#include <core/status.h>
#include <data/cvector.h>
#include <data/cstorage.h>

namespace lsp
{
    enum kvt_param_type_t
    {
        KVT_ANY,                // Any type

        KVT_INT32,
        KVT_UINT32,
        KVT_INT64,
        KVT_UINT64,
        KVT_FLOAT32,            // 32-bit floating-point value
        KVT_FLOAT64,            // 64-bit floating-point value
        KVT_STRING,             // UTF-8 string
        KVT_BLOB                // Binary data
    };

    enum kvt_flags_t
    {
        KVT_COMMIT          = 1 << 0,       // Cleanup modification flag
        KVT_KEEP            = 1 << 1,       // Keep the previous value of parameter if it existed
        KVT_DELEGATE        = 1 << 2        // Delegate the control over parameter's data to the storage
    };

    typedef struct kvt_blob_t
    {
        const char    *ctype;          // Content type
        const void    *data;           // Pointer to data
        size_t         size;           // Size of blob
    } kvt_blob_t;

    typedef struct kvt_param_t
    {
        kvt_param_type_t type;
        union
        {
            int32_t         i32;
            uint32_t        u32;
            int64_t         i64;
            uint64_t        u64;
            float           f32;
            double          f64;
            const char     *str;
            kvt_blob_t      blob;
        };
    } kvt_param_t;

    class KVTStorage;

    class KVTListener
    {
        public:
            explicit KVTListener();
            virtual ~KVTListener();

        public:
            /**
             * Listener has been bound to the storage
             * @param storage storage
             */
            virtual void attached(KVTStorage *storage);

            /**
             * Listener has been unbound from the storage
             * @param storage storage
             */
            virtual void detached(KVTStorage *storage);

            /**
             * Triggers event when parameter has been created
             * @param storage KVT storage that triggered the event
             * @param id the full unique identifier of parameter
             * @param param parameter
             */
            virtual void created(KVTStorage *storage, const char *id, const kvt_param_t *param);

            /**
             * Triggers event when parameter has been rejected (the put() method
             * tried to replace the existing parameter)
             * @param storage KVT storage that triggered the event
             * @param id the full unique identifier of parameter
             * @param rej the rejected parameter
             * @param curr current value parameter
             */
            virtual void rejected(KVTStorage *storage, const char *id, const kvt_param_t *rej, const kvt_param_t *curr);

            /**
             * Triggers event when parameter has been changed
             * @param storage KVT storage that triggered the event
             * @param id the full unique identifier of parameter
             * @param oval old value of parameter
             * @param nval new value of parameter
             */
            virtual void changed(KVTStorage *storage, const char *id, const kvt_param_t *oval, const kvt_param_t *nval);

            /**
             * Triggers event when parameter has been removed
             * @param storage KVT storage that triggered the event
             * @param id the full unique identifier of parameter
             * @param param the value of removed parameter
             */
            virtual void removed(KVTStorage *storage, const char *id, const kvt_param_t *param);

            /**
             * The parameter has been accessed for reading
             * @param storage KVT storage that triggered the event
             * @param id parameter identifier
             * @param param parameter
             */
            virtual void access(KVTStorage *storage, const char *id, const kvt_param_t *param);

            /**
             * The parameter has been committed (it's modification flag has been reset)
             * @param storage KVT storage that triggered the event
             * @param id parameter identifier
             * @param param parameter parameter value
             */
            virtual void commit(KVTStorage *storage, const char *id, const kvt_param_t *param);

            /**
             * The parameter has been accessed for reading/removal
             * @param storage KVT storage that triggered the event
             * @param id parameter identifier
             * @param param parameter
             */
            virtual void missed(KVTStorage *storage, const char *id);
    };

    class KVTIterator;

    /**
     * Key-value tree storage
     */
    class KVTStorage
    {
        private:
            KVTStorage & operator = (const KVTStorage &);

            friend class KVTIterator;

            enum iterator_mode_t
            {
                IT_INVALID,
                IT_MODIFIED,
                IT_BRANCH,
                IT_RECURSIVE,
                IT_EOF
            };

        protected:
            struct kvt_node_t;

            typedef struct kvt_link_t {
                kvt_link_t         *prev;
                kvt_link_t         *next;
                kvt_node_t         *node;
            } kvt_link_t;

            typedef struct kvt_gcparam_t : public kvt_param_t {
                kvt_gcparam_t      *next;
            } kvt_gcparam_t;

            typedef struct kvt_node_t
            {
                char               *id;             // Unique node identifier
                size_t              idlen;          // Length of the ID string
                kvt_node_t         *parent;         // Parent node
                ssize_t             refs;           // Number of references
                kvt_gcparam_t      *param;          // Currently used parameter

                bool                modified;       // Modified flag
                kvt_link_t          gc;             // Link to the removed list
                kvt_link_t          mod;            // Link to the modified list

                kvt_node_t        **children;       // Children
                size_t              nchildren;      // Number of children
                size_t              capacity;       // Capacity in children
            } kvt_node_t;

        protected:
            cvector<KVTListener>    vListeners;

            kvt_link_t              sValid;
            kvt_link_t              sDirty;
            kvt_link_t              sGarbage;
            char                    cSeparator;
            kvt_gcparam_t          *pTrash;
            KVTIterator            *pIterators;
            kvt_node_t              sRoot;
            size_t                  nValues;
            size_t                  nNodes;
            size_t                  nModified;

        protected:
            inline void             notify_created(const char *id, const kvt_param_t *param);
            inline void             notify_rejected(const char *id, const kvt_param_t *rej, const kvt_param_t *curr);
            inline void             notify_changed(const char *id, const kvt_param_t *oval, const kvt_param_t *nval);
            inline void             notify_removed(const char *id, const kvt_param_t *param);
            inline void             notify_access(const char *id, const kvt_param_t *param);
            inline void             notify_commit(const char *id, const kvt_param_t *param);
            inline void             notify_missed(const char *id);

        protected:
            inline static void      link_list(kvt_link_t *root, kvt_link_t *item);
            inline static void      unlink_list(kvt_link_t *item);

            void                    mark_dirty(kvt_node_t *node);
            void                    mark_clean(kvt_node_t *node);
            kvt_node_t             *reference_up(kvt_node_t *node);
            kvt_node_t             *reference_down(kvt_node_t *node);

            char                   *build_path(char **path, size_t *capacity, const kvt_node_t *node);

            void                    destroy_parameter(kvt_gcparam_t *p);
            status_t                commit_parameter(const char *path, kvt_node_t *node, const kvt_param_t *value, size_t flags);
            kvt_gcparam_t          *copy_parameter(const kvt_param_t *src, size_t flags);

            inline void             init_node(kvt_node_t *node, const char *name, size_t len);
            kvt_node_t             *allocate_node(const char *name, size_t len);
            kvt_node_t             *create_node(kvt_node_t *base, const char *name, size_t len);
            void                    destroy_node(kvt_node_t *node);
            kvt_node_t             *get_node(kvt_node_t *base, const char *name, size_t len);
            status_t                walk_node(kvt_node_t **out, const char *name);

            status_t                do_remove_node(const char *name, kvt_node_t *node, const kvt_param_t **value, kvt_param_type_t type);
            status_t                do_touch(const char *name, kvt_node_t *node, bool modified);
            status_t                do_remove_branch(const char *name, kvt_node_t *node);

        public:
            explicit KVTStorage(char separator = '/');
            ~KVTStorage();

            void                    destroy();
            status_t                clear();

        public:
            /**
             * Bind listener to the storage
             * @param listener listener to bind
             * @return status of operation
             */
            status_t    bind(KVTListener *listener);

            /**
             * Unbind listener from storage
             * @param listener listener to unbind
             * @return status of operation
             */
            status_t    unbind(KVTListener *listener);

            /**
             * Check that listener is bound
             * @param listener listener to check
             * @return status of operation
             */
            status_t    is_bound(KVTListener *listener);

            /**
             * Unbind all listeners
             * @return status of operation
             */
            status_t    unbind_all();

        public:
            inline  size_t nodes() const        { return nNodes;    }
            inline  size_t values() const       { return nValues;   }
            inline  size_t modified() const     { return nModified; }
            size_t         listeners() const;

        public:
            /**
             * Put parameter to the storage
             * @param name parameter name
             * @param value parameter value
             * @param flags operation flags @see kvt_flags_t
             * @return status of operation:
             *          STATUS_OK               if parameter has been stored
             *          STATUS_ALREADY_EXISTS   if parameter has been rejected (only if KVT_KEEP is set)
             *          other error             otherwise
             */
            status_t    put(const char *name, const kvt_param_t *value, size_t flags = 0);
            status_t    put(const char *name, uint32_t value, size_t flags = 0);
            status_t    put(const char *name, int32_t value, size_t flags = 0);
            status_t    put(const char *name, uint64_t value, size_t flags = 0);
            status_t    put(const char *name, int64_t value, size_t flags = 0);
            status_t    put(const char *name, float value, size_t flags = 0);
            status_t    put(const char *name, double value, size_t flags = 0);
            status_t    put(const char *name, const char *value, size_t flags = 0);
            status_t    put(const char *name, const kvt_blob_t *value, size_t flags = 0);
            status_t    put(const char *name, size_t size, const char *type, const void *value, size_t flags = 0);

            /**
             * Fetch parameter from the storage
             * @param name parameter name
             * @param value pointer to store the pointer to the parameter
             * @param cast cast the value to the type if possible
             * @return status of operation:
             *          STATUS_OK           if parameter has been successfully read
             *          STATUS_NOT_FOUND    if parameter has been not found
             *          STATUS_BAD_TYPE     if parameter has incompatible type
             */
            status_t    get(const char *name, const kvt_param_t **value, kvt_param_type_t type = KVT_ANY);
            status_t    get(const char *name, uint32_t *value);
            status_t    get(const char *name, int32_t *value);
            status_t    get(const char *name, uint64_t *value);
            status_t    get(const char *name, int64_t *value);
            status_t    get(const char *name, float *value);
            status_t    get(const char *name, double *value);
            status_t    get(const char *name, const char **value);
            status_t    get(const char *name, const kvt_blob_t **value);

            /**
             * Check that parameter of specified type exists
             * @param name parameter name
             * @param type parameter type (optional)
             * @return true if parameter with such type exists
             */
            bool        exists(const char *name, kvt_param_type_t type = KVT_ANY);

            /**
             * Remove parameter from storage
             * @param name parameter name
             * @param value pointer to store the parameter's value
             * @param type value type
             * @return status of operation:
             *          STATUS_OK           if parameter has been removed
             *          STATUS_NOT_FOUND    if parameter has been not found
             */
            status_t    remove(const char *name, const kvt_param_t **value = NULL, kvt_param_type_t type = KVT_ANY);
            status_t    remove(const char *name, uint32_t *value);
            status_t    remove(const char *name, int32_t *value);
            status_t    remove(const char *name, uint64_t *value);
            status_t    remove(const char *name, int64_t *value);
            status_t    remove(const char *name, float *value);
            status_t    remove(const char *name, double *value);
            status_t    remove(const char *name, const char **value);
            status_t    remove(const char *name, const kvt_blob_t **value);

            /**
             * Set the state of node
             * @param name node name
             * @param modified modification flag
             * @return status of operation
             *          STATUS_OK           if parameter has been removed
             *          STATUS_NOT_FOUND    if parameter has been not found
             */
            status_t    touch(const char *name, bool modified = true);
            status_t    commit(const char *name);

            status_t    touch_all(bool modified = true);
            status_t    commit_all();

            /**
             * Remove the full parameter branch from storage
             * @param name parameter name
             * @return status of operation:
             *          STATUS_OK           if parameter has been removed
             *          STATUS_NOT_FOUND    if parameter has been not found
             */
            status_t    remove_branch(const char *name);

            /**
             * Perform garbage collection. Any of previously returned pointers to strings and
             * blobs can become invalid
             * @return status of operation
             */
            status_t    gc();

        public:

            KVTIterator *enum_modified();

            KVTIterator *enum_branch(const char *name, bool recursive = false);
    };

    class KVTIterator
    {
        protected:
            typedef struct kvt_path_t
            {
                KVTStorage::kvt_node_t *node;
                size_t                  index;
            } kvt_path_t;

        protected:
            KVTStorage::kvt_node_t              sFake;
            KVTStorage::iterator_mode_t         enMode;
            KVTStorage::kvt_node_t             *pCurr;
            KVTStorage::kvt_node_t             *pNext;
            size_t                              nIndex;

            cstorage<kvt_path_t>                vPath;
            mutable char                       *pPath;
            mutable char                       *pData;
            mutable size_t                      nDataCap;

            KVTStorage                         *pStorage;
            KVTIterator                        *pGcNext;

            friend class KVTStorage;

        protected:
            KVTIterator & operator = (const KVTIterator &);

            explicit KVTIterator(KVTStorage *storage, KVTStorage::kvt_node_t *node, KVTStorage::iterator_mode_t mode);
            virtual ~KVTIterator();

        public:
            /**
             * Iterate to the next entry
             * @return STATUS_OK if the next entry is present, STATUS_NOT_FOUND if no record found
             */
            status_t    next();
            bool        valid() const;
            bool        modified() const;

        public:

            bool        exists(kvt_param_type_t type = KVT_ANY) const;

            status_t    get(const kvt_param_t **value, kvt_param_type_t type = KVT_ANY);
            status_t    get(uint32_t *value);
            status_t    get(int32_t *value);
            status_t    get(uint64_t *value);
            status_t    get(int64_t *value);
            status_t    get(float *value);
            status_t    get(double *value);
            status_t    get(const char **value);
            status_t    get(const kvt_blob_t **value);

            status_t    put(const kvt_param_t *value, size_t flags = 0);
            status_t    put(uint32_t value, size_t flags = 0);
            status_t    put(int32_t value, size_t flags = 0);
            status_t    put(uint64_t value, size_t flags = 0);
            status_t    put(int64_t value, size_t flags = 0);
            status_t    put(float value, size_t flags = 0);
            status_t    put(double value, size_t flags = 0);
            status_t    put(const char *value, size_t flags = 0);
            status_t    put(const kvt_blob_t *value, size_t flags = 0);
            status_t    put(size_t size, const char *type, const void *value, size_t flags = 0);

            status_t    remove(const kvt_param_t **value = NULL, kvt_param_type_t type = KVT_ANY);
            status_t    remove(uint32_t *value);
            status_t    remove(int32_t *value);
            status_t    remove(uint64_t *value);
            status_t    remove(int64_t *value);
            status_t    remove(float *value);
            status_t    remove(double *value);
            status_t    remove(const char **value);
            status_t    remove(const kvt_blob_t **value);

            status_t    touch(bool modified = true);
            status_t    commit();

            status_t    remove_branch();

            const char *name() const;
            const char *id() const;
    };


} /* namespace lsp */

#endif /* INCLUDE_CORE_KVTSTORAGE_H_ */