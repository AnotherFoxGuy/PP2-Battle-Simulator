'use strict';

module.exports = {
    writerOpts: {
        mainTemplate: `{{#each commitGroups}}{{#each commits}}{{> commit root=@root}}{{/each}}{{/each}}`,
        commitPartial: `{{#if body}}\n\n
{{body}}{{/if}}`
    }
};
